#include "ObjectStorePrivate.h"

#include "Exceptions.h"
#include "Message.h"
#include "js/MessageManagerPrivate.h"
#include "js/ObjectWatcherPrivate.h"
#include "js/Publisher.h"
#include "js/StreamPrivate.h"
#include "js/SyncSubscriptionPrivate.h"
#include "private/utils.h"

using namespace NatsMq;

namespace
{
    constexpr auto defaultChunkSize = 128ull * 1024ull; // 128 kB

    static const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    std::string base64Encode(const std::string& input)
    {
        std::string output;
        int         val  = 0;
        int         valb = -6;
        for (unsigned char c : input)
        {
            val = (val << 8) + c;
            valb += 8;
            while (valb >= 0)
            {
                output.push_back(base64_chars[(val >> valb) & 0x3F]);
                valb -= 6;
            }
        }
        if (valb > -6)
        {
            output.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
        }
        while (output.size() % 4)
        {
            output.push_back('=');
        }
        return output;
    }

    std::string objectStreamNameTemplate(const std::string& bucket)
    {
        return "OBJ_" + bucket;
    }

    std::string objectAllChunksPreTemplate(const std::string& bucket)
    {
        return "$O." + bucket + ".C.>";
    }

    std::string objectChunksPreTemplate(const std::string& bucket, const std::string& nuid)
    {
        return "$O." + bucket + ".C." + nuid;
    }

    std::string objectAllMetaPreTemplate(const std::string& bucket)
    {
        return "$O." + bucket + ".M.>";
    }

    std::string objectMetaPreTemplate(const std::string& bucket, const std::string& obj)
    {
        return "$O." + bucket + ".M." + base64Encode(obj);
    }

    Js::StreamConfig createStreamConfig(const Js::ObjectStoreConfig& config)
    {
        Js::StreamConfig streamConfig;

        streamConfig.name         = objectStreamNameTemplate(config.bucket);
        streamConfig.description  = config.description;
        streamConfig.storage      = config.storage;
        streamConfig.replicas     = config.replicas;
        streamConfig.maxBytes     = config.maxBytes;
        streamConfig.maxAge       = config.maxAge;
        streamConfig.discard      = Js::DiscardPolicy::FailStoreMessage;
        streamConfig.maxConsumers = 0;
        streamConfig.allowDirect  = true;
        streamConfig.allowRollup  = true;
        streamConfig.subjects     = { objectAllChunksPreTemplate(config.bucket), objectAllMetaPreTemplate(config.bucket) };

        return streamConfig;
    }

    Js::ObjectStoreConfig streamInfoToConfig(const Js::StreamInfo& info, const std::string& bucket)
    {
        Js::ObjectStoreConfig config;
        config.bucket      = bucket;
        config.description = info.config.description;
        config.maxAge      = info.config.maxAge;
        config.maxBytes    = info.config.maxBytes;
        config.replicas    = info.config.replicas;
        config.storage     = info.config.storage;
        return config;
    }
}

bool Js::ObjectStorePrivate::exists(jsCtx* ctx, const std::string& bucket)
{
    return Js::StreamPrivate::exists(ctx, objectStreamNameTemplate(bucket));
}

NatsMq::Js::ObjectStorePrivate::ObjectStorePrivate(jsCtx* ctx, const ObjectStoreConfig& config)
    : _ctx(ctx)
    , _bucket(config.bucket)
{
    if (!exists(ctx, config.bucket))
    {
        const auto streamName = objectStreamNameTemplate(config.bucket);

        Js::StreamPrivate stream(ctx, streamName);
        stream.create(createStreamConfig(config));
    }
}

Js::ObjectInfo Js::ObjectStorePrivate::info(const std::string& name) const
{
    const auto streamName  = objectStreamNameTemplate(_bucket);
    const auto metaSubject = objectMetaPreTemplate(_bucket, name);

    MessageManagerPrivate messageMngr(_ctx);

    const auto message = messageMngr.getLastMessage(streamName, metaSubject);

    return deserializeObjectMeta(message);
}

Js::ObjectElement Js::ObjectStorePrivate::get(const std::string& name, int64_t chunkTimeoutMs) const
{
    auto meta = info(name);

    const auto streamName    = objectStreamNameTemplate(_bucket);
    const auto chunksSubject = objectChunksPreTemplate(_bucket, meta.uid);

    Js::SubscriptionOptions options;
    options.ordered = true;
    options.stream  = streamName;

    Js::SyncSubscriptionPrivate subscription(_ctx, chunksSubject, options);

    try
    {
        // TODO check data integrity? crc or other

        Js::ObjectElement result;
        result.data.reserve(meta.size);

        for (auto i = 0; i < meta.chunks; ++i)
        {
            auto msg = subscription.next(chunkTimeoutMs);
            result.data.insert(result.data.end(), std::make_move_iterator(msg.msg.data.begin()), std::make_move_iterator(msg.msg.data.end()));
        }

        result.meta = std::move(meta);

        return result;
    }
    catch (const std::bad_alloc&)
    {
        throw JsException(NatsMq::Status::NoMemmory, Js::Status::NoJsError);
    }
}

void Js::ObjectStorePrivate::put(const ObjectElement& object) const
{
    if (object.data.empty() || object.meta.name.empty())
        throw JsException(NatsMq::Status::InvalidArg, Js::Status::NoJsError);

    const auto streamName    = objectStreamNameTemplate(_bucket);
    const auto metaSubject   = objectMetaPreTemplate(_bucket, object.meta.name);
    const auto chunksSubject = objectChunksPreTemplate(_bucket, object.meta.uid);

    auto meta    = object.meta;
    meta.chunks  = 0;
    meta.size    = object.data.size();
    meta.bucket  = _bucket;
    meta.deleted = false;

    Js::Publisher publisher(_ctx);

    // TODO check valid
    auto pos{ 0ull }, chunkSize{ 0ull };
    for (auto it = object.data.begin(); it != object.data.end(); std::advance(it, chunkSize))
    {
        chunkSize = std::min(defaultChunkSize, object.data.size() - pos);
        pos += chunkSize;

        auto ite = object.data.begin();
        std::advance(ite, pos);

        Message msg;
        msg.subject = chunksSubject;
        msg.data.insert(msg.data.end(), it, ite);

        publisher.publish(std::move(msg), {});

        ++meta.chunks;
    }

    Message msg;
    msg.subject = metaSubject;
    msg.data    = serializeObjectMeta(meta);

    publisher.publish(std::move(msg), {});
}

void Js::ObjectStorePrivate::remove(const std::string& name) const
{
    auto meta = info(name);

    const auto streamName    = objectStreamNameTemplate(_bucket);
    const auto metaSubject   = objectMetaPreTemplate(_bucket, name);
    const auto chunksSubject = objectChunksPreTemplate(_bucket, meta.uid);

    Js::StreamPrivate stream(_ctx, streamName);

    Js::Options::Stream::Purge opts;
    opts.subject = chunksSubject;
    stream.purge(opts);

    Js::Publisher publisher(_ctx);

    meta.name    = name;
    meta.size    = 0;
    meta.chunks  = 0;
    meta.uid     = "";
    meta.deleted = true;

    Message msg;
    msg.subject = metaSubject;
    msg.data    = serializeObjectMeta(meta);

    publisher.publish(std::move(msg), {});
}

Js::ObjectWatcherPrivate* Js::ObjectStorePrivate::watch(const std::string& name) const
{
    return new ObjectWatcherPrivate(_ctx, objectMetaPreTemplate(_bucket, name), objectStreamNameTemplate(_bucket));
}

bool Js::ObjectStorePrivate::storeExists() const
{
    return exists(_ctx, _bucket);
}

void Js::ObjectStorePrivate::deleteStore() const
{
    const auto streamName = objectStreamNameTemplate(_bucket);

    Js::StreamPrivate stream(_ctx, streamName);

    stream.remove();
}

Js::ObjectStoreConfig Js::ObjectStorePrivate::storeConfig() const
{
    const auto streamName = objectStreamNameTemplate(_bucket);

    const auto info = Js::StreamPrivate::info(_ctx, streamName);

    return streamInfoToConfig(info, _bucket);
}
