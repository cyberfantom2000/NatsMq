#include "KeyValueStorePrivate.h"

#include "Entities.h"
#include "Exceptions.h"

using namespace NatsMq;

namespace
{
    kvConfig toCnatsKVConfig(const Js::KeyValue::Config& config)
    {

        kvConfig cfg;
        kvConfig_Init(&cfg);
        cfg.Bucket       = config.bucket.c_str();
        cfg.Description  = config.description.c_str();
        cfg.MaxValueSize = config.maxValueSize;
        cfg.History      = config.history;
        cfg.TTL          = config.ttl;
        cfg.MaxBytes     = config.maxBytes;
        cfg.StorageType  = static_cast<decltype(cfg.StorageType)>(config.storageType);
        cfg.Replicas     = config.replicas;

        // WARNING Not testing
        if (!config.republish.destination.empty() && !config.republish.source.empty())
        {
            cfg.RePublish = new jsRePublish;

            cfg.RePublish->Destination = config.republish.destination.c_str();
            cfg.RePublish->Source      = config.republish.source.c_str();
            cfg.RePublish->HeadersOnly = config.republish.headersOnly;
        }

        auto setStreamSource = [](jsStreamSource* src, const Js::StreamSource& source) {
            src->Name          = source.name.c_str();
            src->Domain        = source.domain.c_str();
            src->FilterSubject = source.filterSubject.c_str();
            src->Domain        = source.domain.c_str();
            src->OptStartSeq   = source.optStartSequence;
            src->OptStartTime  = source.optStartTime;

            src->External = new jsExternalStream;

            src->External->APIPrefix     = source.external.APIPrefix.c_str();
            src->External->DeliverPrefix = source.external.deliverPrefix.c_str();
        };

        // WARNING Not testing
        if (!config.mirror.name.empty() && !config.mirror.domain.empty())
        {
            cfg.Mirror = new jsStreamSource;
            setStreamSource(cfg.Mirror, config.mirror);
        }

        // WARNING Not testing
        if (!config.sources.empty())
        {
            auto sources = new jsStreamSource[config.sources.size()];
            cfg.Sources  = &sources;

            setStreamSource(cfg.Mirror, config.mirror);
            for (auto i = 0u; i < config.sources.size(); ++i)
                setStreamSource(cfg.Sources[i], config.sources[i]);

            cfg.SourcesLen = config.sources.size();
        }

        return cfg;
    }

    void cnatcKvConfigDestroy(kvConfig* conf)
    {
        delete conf->RePublish;
        delete conf->Mirror;
        //            delete[] conf->Sources;
    }
}

bool Js::KeyValueStorePrivate::exists(jsCtx* ctx, const std::string& bucket)
{
    kvStore* store{ nullptr };

    const auto status = js_KeyValue(&store, ctx, bucket.c_str());

    NatsKvPtr ptr(store, &kvStore_Destroy);

    if (static_cast<NatsMq::Status>(status) == NatsMq::Status::NotFound)
        return false;

    jsExceptionIfError(status);

    return true;
}

Js::KeyValueStorePrivate::KeyValueStorePrivate(jsCtx* ctx, const Js::KeyValue::Config& config)
    : _ctx(ctx)
    , _kv(nullptr, &kvStore_Destroy)
{
    kvStore* kv{ nullptr };
    if (exists(ctx, config.bucket))
    {
        const auto status = js_KeyValue(&kv, ctx, config.bucket.c_str());
        jsExceptionIfError(status);
    }
    else
    {
        auto kvConf = toCnatsKVConfig(config);

        const auto status = js_CreateKeyValue(&kv, ctx, &kvConf);

        cnatcKvConfigDestroy(&kvConf);

        jsExceptionIfError(status);
    }

    _kv.reset(kv);
}

std::string Js::KeyValueStorePrivate::bucket() const noexcept
{
    return kvStore_Bucket(_kv.get());
}

Js::KeyValue::Info Js::KeyValueStorePrivate::info() const
{
    kvStatus* status{ nullptr };
    jsExceptionIfError(kvStore_Status(&status, _kv.get()));

    KeyValue::Info result;
    result.bucket   = kvStatus_Bucket(status);
    result.values   = kvStatus_Values(status);
    result.bytes    = kvStatus_Bytes(status);
    result.history  = kvStatus_History(status);
    result.replicas = kvStatus_Replicas(status);
    result.ttl      = kvStatus_TTL(status);

    kvStatus_Destroy(status);

    return result;
}

std::vector<std::string> Js::KeyValueStorePrivate::keys() const
{
    kvKeysList list;
    jsExceptionIfError(kvStore_Keys(&list, _kv.get(), nullptr));

    std::vector<std::string> out;
    for (auto i = 0; i < list.Count; ++i)
        out.emplace_back(list.Keys[i]);

    kvKeysList_Destroy(&list);
    return out;
}

Js::KeyValue::Element Js::KeyValueStorePrivate::get(const std::string& key) const
{
    const auto entryPtr = getKvEntry(key);
    const auto entry    = entryPtr.get();

    const auto data = reinterpret_cast<const uint8_t*>(kvEntry_Value(entry));
    const auto len  = kvEntry_ValueLen(entry);

    KeyValue::Element result;
    result.bucket = kvEntry_Bucket(entry);
    result.key    = kvEntry_Key(entry);
    result.value.reserve(len);
    std::copy(data, data + len, std::back_inserter(result.value));
    result.revision = kvEntry_Revision(entry);
    result.created  = kvEntry_Created(entry);
    result.delta    = kvEntry_Delta(entry);

    return result;
}

std::string Js::KeyValueStorePrivate::getString(const std::string& key) const
{
    const auto  entry = getKvEntry(key);
    std::string result(kvEntry_ValueString(entry.get()));
    return result;
}

void Js::KeyValueStorePrivate::put(const std::string& key, const std::vector<uint8_t>& value, uint64_t revision) const
{
    jsExceptionIfError(kvStore_Put(&revision, _kv.get(), key.c_str(), value.data(), static_cast<int>(value.size())));
}

void Js::KeyValueStorePrivate::put(const std::string& key, const std::string& value, uint64_t revision) const
{
    jsExceptionIfError(kvStore_PutString(&revision, _kv.get(), key.c_str(), value.c_str()));
}

void Js::KeyValueStorePrivate::create(const std::string& key, const std::vector<uint8_t>& value, uint64_t revision) const
{
    jsExceptionIfError(kvStore_Create(&revision, _kv.get(), key.c_str(), value.data(), static_cast<int>(value.size())));
}

void Js::KeyValueStorePrivate::create(const std::string& key, const std::string& value, uint64_t revision) const
{
    jsExceptionIfError(kvStore_CreateString(&revision, _kv.get(), key.c_str(), value.c_str()));
}

void Js::KeyValueStorePrivate::update(const std::string& key, const std::vector<uint8_t>& value, uint64_t newRevision, uint64_t expectedRevision)
{
    jsExceptionIfError(kvStore_Update(&newRevision, _kv.get(), key.c_str(), value.data(), static_cast<int>(value.size()), expectedRevision));
}

void Js::KeyValueStorePrivate::update(const std::string& key, const std::string& value, uint64_t newRevision, uint64_t expectedRevision)
{
    jsExceptionIfError(kvStore_UpdateString(&newRevision, _kv.get(), key.c_str(), value.c_str(), expectedRevision));
}

void Js::KeyValueStorePrivate::purge(const std::string& key) const
{
    jsExceptionIfError(kvStore_Purge(_kv.get(), key.c_str(), nullptr));
}

void Js::KeyValueStorePrivate::remove(const std::string& key) const
{
    jsExceptionIfError(kvStore_Delete(_kv.get(), key.c_str()));
}

void Js::KeyValueStorePrivate::deleteStore() const
{
    const auto name = bucket();
    jsExceptionIfError(js_DeleteKeyValue(_ctx, name.c_str()));
}

Js::KvEntryPtr Js::KeyValueStorePrivate::getKvEntry(const std::string& key) const
{
    kvEntry* natsEntry{ nullptr };
    jsExceptionIfError(kvStore_Get(&natsEntry, _kv.get(), key.c_str()));
    KvEntryPtr entry(natsEntry, &kvEntry_Destroy);
    return entry;
}
