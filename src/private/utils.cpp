#include "utils.h"

#include <random>
#include <sstream>

#include "Exceptions.h"
#include "Message.h"
#include "picojson.h"

namespace
{

    const char* ptrIfNotEmpty(const std::string& str)
    {
        return str.empty() ? nullptr : str.c_str();
    }

    NatsMq::Message::Headers parseHeaders(natsMsg* msg)
    {
        const char** keys{ nullptr };
        int          count;

        auto status = natsMsgHeader_Keys(msg, &keys, &count);
        if (status != NATS_OK)
            return {};

        NatsMq::Message::Headers headers;
        for (auto i = 0; i < count; ++i)
        {
            const char* value;
            natsMsgHeader_Get(msg, keys[i], &value);
            headers[keys[i]] = value;
        }

        free((void*)keys);
        return headers;
    }

}

NatsMq::NatsMsgPtr NatsMq::createCnatsMessage(const NatsMq::Message& msg)
{
    natsMsg* cnatsMsg;

    const auto realReply = ptrIfNotEmpty(msg.replySubject);

    const auto dataSize = static_cast<int>(msg.data.size());

    const auto dataPtr = reinterpret_cast<const char*>(msg.data.data());

    exceptionIfError(natsMsg_Create(&cnatsMsg, msg.subject.c_str(), realReply, dataPtr, dataSize));

    NatsMsgPtr natsMsg(cnatsMsg, &natsMsg_Destroy);

    for (auto it = msg.headers.cbegin(), ite = msg.headers.cend(); it != ite; ++it)
        exceptionIfError(natsMsgHeader_Add(cnatsMsg, it->first.c_str(), it->second.c_str()));

    return natsMsg;
}

NatsMq::Message NatsMq::fromCnatsMessage(natsMsg* msg)
{
    if (!msg)
        return {};

    NatsMq::Message out;
    out.subject      = emptyStringIfNull(natsMsg_GetSubject(msg));
    out.replySubject = emptyStringIfNull(natsMsg_GetReply(msg));

    const auto dataPtr = natsMsg_GetData(msg);
    const auto dataLen = natsMsg_GetDataLength(msg);

    out.data    = decltype(out.data)(dataPtr, dataPtr + dataLen);
    out.headers = parseHeaders(msg);

    return out;
}

bool NatsMq::makePing(natsConnection* connection, int timeout)
{
    const auto status = static_cast<Status>(natsConnection_FlushTimeout(connection, timeout));
    return status == Status::Ok;
}

void NatsMq::configurePoolSize(int poolSize)
{
    if (poolSize > 1)
        nats_SetMessageDeliveryPoolSize(poolSize);
}

std::string NatsMq::emptyStringIfNull(const char* s)
{
    return s ? s : "";
}

std::vector<const char*> NatsMq::createArrayPointersToElements(const std::vector<std::string>& elements)
{
    std::vector<const char*> pointers;
    for (auto&& url : elements)
        pointers.push_back(url.c_str());

    return pointers;
}

natsMetadata NatsMq::toNatsMetadata(std::vector<const char*>& data)
{
    natsMetadata meta;
    meta.List  = data.data();
    meta.Count = data.size();
    return meta;
}

std::string Utils::uuid()
{
    static std::random_device dev;
    static std::mt19937       rng(dev());

    std::uniform_int_distribution<int> dist(0, 15);

    const char* v      = "0123456789abcdef";
    const bool  dash[] = { 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0 };

    std::string res;
    for (int i = 0; i < 16; i++)
    {
        if (dash[i])
            res += "-";
        res += v[dist(rng)];
        res += v[dist(rng)];
    }

    return res;
}

jsSubOptions NatsMq::Js::toJsCnatsSubOptions(const SubscriptionOptions& options)
{
    jsSubOptions jsOpt;
    jsSubOptions_Init(&jsOpt);

    jsOpt.Consumer  = ptrIfNotEmpty(options.consumer);
    jsOpt.Queue     = ptrIfNotEmpty(options.queue);
    jsOpt.Stream    = ptrIfNotEmpty(options.stream);
    jsOpt.ManualAck = options.manualAck;
    jsOpt.Ordered   = options.ordered;
    jsOpt.Config    = toJsConsumerConfig(options.config);

    return jsOpt;
}

jsConsumerConfig NatsMq::Js::toJsConsumerConfig(const ConsumerConfig& config)
{
    jsConsumerConfig jsCfg;

    jsConsumerConfig_Init(&jsCfg);

    jsCfg.Name               = ptrIfNotEmpty(config.name);
    jsCfg.Durable            = ptrIfNotEmpty(config.durable);
    jsCfg.Description        = ptrIfNotEmpty(config.description);
    jsCfg.FilterSubject      = ptrIfNotEmpty(config.filterSubject);
    jsCfg.SampleFrequency    = ptrIfNotEmpty(config.sampleFrequncy);
    jsCfg.DeliverSubject     = ptrIfNotEmpty(config.deliverSubject);
    jsCfg.DeliverGroup       = ptrIfNotEmpty(config.deliverGroup);
    jsCfg.DeliverPolicy      = static_cast<decltype(jsCfg.DeliverPolicy)>(config.deliverPolicy);
    jsCfg.AckPolicy          = static_cast<decltype(jsCfg.AckPolicy)>(config.ackPolicy);
    jsCfg.ReplayPolicy       = static_cast<decltype(jsCfg.ReplayPolicy)>(config.replayPolicy);
    jsCfg.OptStartSeq        = config.optStartSeq;
    jsCfg.OptStartTime       = config.optStartTime;
    jsCfg.AckWait            = config.ackWait;
    jsCfg.MaxDeliver         = config.maxDeliver;
    jsCfg.MaxAckPending      = config.maxAckPending;
    jsCfg.MaxWaiting         = config.maxWaiting;
    jsCfg.RateLimit          = config.rateLimit;
    jsCfg.Heartbeat          = config.heartbeat;
    jsCfg.MaxRequestBatch    = config.maxRequestBatch;
    jsCfg.MaxRequestExpires  = config.maxRequestExpires;
    jsCfg.MaxRequestMaxBytes = config.maxRequestMaxBytes;
    jsCfg.Replicas           = config.replicas;
    jsCfg.BackOff            = const_cast<int64_t*>(config.backOff.data());
    jsCfg.BackOffLen         = config.backOff.size();
    jsCfg.FlowControl        = config.flowControl;
    jsCfg.HeadersOnly        = config.headersOnly;
    jsCfg.MemoryStorage      = config.memoryStorage;

    return jsCfg;
}

NatsMq::Js::ConsumerConfig NatsMq::Js::fromCnatsConsumerConfig(jsConsumerConfig* cfg)
{
    Js::ConsumerConfig result;

    result.name               = emptyStringIfNull(cfg->Name);
    result.durable            = emptyStringIfNull(cfg->Durable);
    result.description        = emptyStringIfNull(cfg->Description);
    result.filterSubject      = emptyStringIfNull(cfg->FilterSubject);
    result.deliverSubject     = emptyStringIfNull(cfg->DeliverSubject);
    result.deliverGroup       = emptyStringIfNull(cfg->DeliverGroup);
    result.deliverPolicy      = static_cast<decltype(result.deliverPolicy)>(cfg->DeliverPolicy);
    result.ackPolicy          = static_cast<decltype(result.ackPolicy)>(cfg->AckPolicy);
    result.replayPolicy       = static_cast<decltype(result.replayPolicy)>(cfg->ReplayPolicy);
    result.optStartSeq        = cfg->OptStartSeq;
    result.optStartTime       = cfg->OptStartTime;
    result.ackWait            = cfg->AckWait;
    result.maxDeliver         = cfg->MaxDeliver;
    result.maxAckPending      = cfg->MaxAckPending;
    result.maxWaiting         = cfg->MaxWaiting;
    result.rateLimit          = cfg->RateLimit;
    result.heartbeat          = cfg->Heartbeat;
    result.maxRequestBatch    = cfg->MaxRequestBatch;
    result.maxRequestExpires  = cfg->MaxRequestExpires;
    result.maxRequestMaxBytes = cfg->MaxRequestMaxBytes;
    result.inactiveTreshold   = cfg->InactiveThreshold;
    result.replicas           = cfg->Replicas;

    for (auto i = 0; i < cfg->BackOffLen; ++i)
        result.backOff.push_back(cfg->BackOff[i]);

    result.flowControl   = cfg->FlowControl;
    result.headersOnly   = cfg->HeadersOnly;
    result.memoryStorage = cfg->MemoryStorage;

    return result;
}

std::vector<uint8_t> NatsMq::Js::serializeObjectMeta(const ObjectInfo& info)
{
    picojson::value::object obj;
    obj["bucket"]  = picojson::value(info.bucket);
    obj["name"]    = picojson::value(info.name);
    obj["size"]    = picojson::value(info.size);
    obj["nuid"]    = picojson::value(info.uid);
    obj["chunks"]  = picojson::value(info.chunks);
    obj["deleted"] = picojson::value(info.deleted);

    picojson::value json(obj);

    const auto str = json.serialize();

    std::vector<uint8_t> buf;
    buf.insert(buf.begin(), str.begin(), str.end());
    return buf;
}

NatsMq::Js::ObjectInfo NatsMq::Js::deserializeObjectMeta(const NatsMq::Message& msg)
{
    std::string        str(reinterpret_cast<const char*>(msg.data.data()), msg.data.size());
    std::istringstream iss(str);

    picojson::value json;
    iss >> json;

    const auto err = picojson::get_last_error();
    if (!err.empty() || !json.is<picojson::object>())
        jsExceptionIfError(NatsMq::Status::Error, NatsMq::Js::Status::InvalidJSONErr);

    NatsMq::Js::ObjectInfo info;
    info.bucket  = json.get("bucket").get<std::string>();
    info.name    = json.get("name").get<std::string>();
    info.size    = json.get("size").get<int64_t>();
    info.uid     = json.get("nuid").get<std::string>();
    info.chunks  = json.get("chunks").get<int64_t>();
    info.deleted = json.get("deleted").get<bool>();

    return info;
}
