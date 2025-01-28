#include "StreamPrivate.h"

#include <memory>

#include "Exceptions.h"

using namespace NatsMq::Js;

namespace
{
    using StreamInfoPtr      = std::unique_ptr<jsStreamInfo, decltype(&jsStreamInfo_Destroy)>;
    using StreamInfoListPtr  = std::unique_ptr<jsStreamInfoList, decltype(&jsStreamInfoList_Destroy)>;
    using StreamNamesListPtr = std::unique_ptr<jsStreamNamesList, decltype(&jsStreamNamesList_Destroy)>;

    void setIfNotNull(std::string& s, const char* cstr)
    {
        if (cstr)
            s = cstr;
    }

    std::vector<const char*> createElementPointers(const std::vector<std::string>& elements)
    {
        std::vector<const char*> pointers;
        for (auto&& element : elements)
            pointers.push_back(element.c_str());
        return pointers;
    }

    jsStreamConfig toCNatsConfig(const NatsMq::Js::StreamConfig& newConfig, std::vector<const char*>& subjects)
    {
        jsStreamConfig config;
        jsStreamConfig_Init(&config);

        config.Name        = newConfig.name.c_str();
        config.Description = newConfig.description.c_str();
        config.Subjects    = subjects.data();
        config.SubjectsLen = static_cast<int>(subjects.size());

        config.AllowDirect  = newConfig.allowDirect;
        config.AllowRollup  = newConfig.allowRollup;
        config.MirrorDirect = newConfig.mirrorDirect;
        config.NoAck        = newConfig.noAck;
        config.Sealed       = newConfig.sealed;
        config.DenyDelete   = newConfig.denyDelete;
        config.DenyPurge    = newConfig.denyPurge;

        config.Storage     = static_cast<jsStorageType>(newConfig.storage);
        config.Discard     = static_cast<jsDiscardPolicy>(newConfig.discard);
        config.Retention   = static_cast<jsRetentionPolicy>(newConfig.retention);
        config.Compression = static_cast<jsStorageCompression>(newConfig.compression);

        config.MaxMsgs           = newConfig.maxMessages;
        config.MaxMsgsPerSubject = newConfig.maxMessagesPerSubject;
        config.MaxMsgSize        = newConfig.maxMessageSize;
        config.MaxAge            = newConfig.maxAge * 1000000000;
        config.MaxBytes          = newConfig.maxBytes;
        config.MaxConsumers      = newConfig.maxConsumers;
        config.Replicas          = newConfig.replicas;
        config.Duplicates        = newConfig.duplicateWindow;

        return config;
    }

    jsRePublish toJsRepublish(const NatsMq::Js::StreamConfig& newConfig)
    {
        jsRePublish republish;
        jsRePublish_Init(&republish);
        republish.Source      = newConfig.republish.source.c_str();
        republish.Destination = newConfig.republish.destination.c_str();
        republish.HeadersOnly = newConfig.republish.headersOnly;
        return republish;
    }

    jsOptions toJsOptions(const NatsMq::Js::Options::Stream::Purge& opt)
    {
        if (opt.sequence && opt.keep)
            throw NatsMq::JsException(NatsMq::Status::InvalidArg, NatsMq::Js::Status::NoJsError);

        jsOptions jsOptions;
        jsOptions_Init(&jsOptions);

        jsOptions.Stream.Purge.Subject  = opt.subject.empty() ? nullptr : opt.subject.c_str();
        jsOptions.Stream.Purge.Keep     = opt.keep;
        jsOptions.Stream.Purge.Sequence = opt.sequence;

        return jsOptions;
    }

    NatsMq::Js::StreamConfig fromCnatsConfig(jsStreamConfig* config)
    {
        NatsMq::Js::StreamConfig result;

        setIfNotNull(result.name, config->Name);
        setIfNotNull(result.description, config->Description);

        for (auto i = 0; i < config->SubjectsLen; ++i)
            result.subjects.emplace_back(config->Subjects[i]);

        result.allowDirect  = config->AllowDirect;
        result.allowRollup  = config->AllowRollup;
        result.mirrorDirect = config->MirrorDirect;
        result.noAck        = config->NoAck;
        result.sealed       = config->Sealed;
        result.denyDelete   = config->DenyDelete;
        result.denyPurge    = config->DenyPurge;

        if (config->RePublish)
        {
            result.republish.source      = config->RePublish->Source;
            result.republish.destination = config->RePublish->Destination;
            result.republish.headersOnly = config->RePublish->HeadersOnly;
        }

        result.storage     = static_cast<decltype(result.storage)>(config->Storage);
        result.discard     = static_cast<decltype(result.discard)>(config->Discard);
        result.retention   = static_cast<decltype(result.retention)>(config->Retention);
        result.compression = static_cast<decltype(result.compression)>(config->Compression);

        result.maxMessages           = config->MaxMsgs;
        result.maxMessagesPerSubject = config->MaxMsgsPerSubject;
        result.maxMessageSize        = config->MaxMsgSize;
        result.maxAge                = config->MaxAge / 1000000000;
        result.maxBytes              = config->MaxBytes;
        result.maxConsumers          = config->MaxConsumers;
        result.replicas              = config->Replicas;
        result.duplicateWindow       = config->Duplicates;

        return result;
    }

    NatsMq::Js::StreamState fromCnatsState(jsStreamState* state)
    {
        NatsMq::Js::StreamState result;

        result.messages      = state->Msgs;
        result.bytes         = state->Bytes;
        result.firstSequence = state->FirstSeq;
        result.lastSequence  = state->LastSeq;
        result.firstTime     = state->FirstTime;
        result.lastTime      = state->LastTime;

        if (state->Subjects)
        {
            for (auto i = 0; i < state->Subjects->Count; ++i)
            {
                auto subjState = state->Subjects->List[i];
                result.subjects.push_back({ subjState.Subject, subjState.Msgs });
            }
        }

        for (auto i = 0; i < state->DeletedLen; ++i)
            result.deleted.push_back(state->Deleted[i]);

        if (state->Lost)
        {
            result.lostData.bytes = state->Lost->Bytes;

            for (auto i = 0; i < state->Lost->MsgsLen; ++i)
                result.lostData.messages.push_back(state->Lost->Msgs[i]);
        }

        result.consumers = state->Consumers;

        return result;
    }

    NatsMq::Js::StreamCluster fromCnatsCluster(jsClusterInfo* cluster)
    {
        NatsMq::Js::StreamCluster result;

        setIfNotNull(result.name, cluster->Name);
        setIfNotNull(result.leader, cluster->Leader);

        for (auto i = 0; i < cluster->ReplicasLen; ++i)
        {
            auto cnatsReplica = cluster->Replicas[i];
            result.replicas.push_back({ cnatsReplica->Current, cnatsReplica->Offline,
                                        cnatsReplica->Name, cnatsReplica->Active,
                                        cnatsReplica->Lag });
        }

        return result;
    }

    NatsMq::Js::StreamSourceInfo fromCnatcSource(jsStreamSourceInfo* src)
    {
        NatsMq::Js::StreamSourceInfo result;

        result.name          = src->Name;
        result.lag           = src->Lag;
        result.active        = src->Active;
        result.filterSubject = src->FilterSubject;

        result.external.APIPrefix     = src->External->APIPrefix;
        result.external.deliverPrefix = src->External->DeliverPrefix;

        for (auto i = 0; i < src->SubjectTransformsLen; ++i)
        {
            const auto cnatsSubjTrans = src->SubjectTransforms[i];
            result.subjectTransforms.push_back({ cnatsSubjTrans.Source, cnatsSubjTrans.Destination });
        }

        return result;
    }

    NatsMq::Js::StreamAlternate fromCnatsAlternat(jsStreamAlternate* alt)
    {
        return { alt->Name, alt->Domain, alt->Cluster };
    }

    NatsMq::Js::StreamInfo fromCnatsInfo(jsStreamInfo* info)
    {
        NatsMq::Js::StreamInfo result;

        if (info->Config)
            result.config = fromCnatsConfig(info->Config);

        result.createdNs = result.createdNs;
        result.state     = fromCnatsState(&info->State);

        if (info->Cluster)
            result.cluster = fromCnatsCluster(info->Cluster);

        if (info->Mirror)
            result.mirror = fromCnatcSource(info->Mirror);

        for (auto i = 0; i < info->SourcesLen; ++i)
            result.sources.push_back(fromCnatcSource(info->Sources[i]));

        for (auto i = 0; i < info->AlternatesLen; ++i)
            result.alternates.push_back(fromCnatsAlternat(info->Alternates[i]));

        return result;
    }
}

StreamPrivate::StreamPrivate(jsCtx* context, const std::string& name) noexcept
    : _name(name)
    , _context(context)
{
}

void StreamPrivate::create(const StreamConfig& config) const
{
    auto subjects        = createElementPointers(config.subjects);
    auto natsRepublish   = toJsRepublish(config);
    auto natsConfig      = toCNatsConfig(config, subjects);
    natsConfig.RePublish = &natsRepublish;

    jsErrCode  jerr;
    const auto status = js_AddStream(nullptr, _context, &natsConfig, nullptr, &jerr);

    jsExceptionIfError(status, jerr);
}

bool StreamPrivate::exists() const
{
    return StreamPrivate::exists(_context, _name);
}

StreamInfo StreamPrivate::info() const
{
    return StreamPrivate::info(_context, _name);
}

void StreamPrivate::purge(const Js::Options::Stream::Purge& options) const
{
    auto jsOptions = toJsOptions(options);

    jsErrCode  jerr;
    const auto status = js_PurgeStream(_context, _name.c_str(), &jsOptions, &jerr);

    jsExceptionIfError(status, jerr);
}

void StreamPrivate::remove() const
{
    jsErrCode  jerr;
    const auto status = js_DeleteStream(_context, _name.c_str(), nullptr, &jerr);
    jsExceptionIfError(status, jerr);
}

void StreamPrivate::update(const StreamConfig& config) const
{
    auto subjects        = createElementPointers(config.subjects);
    auto natsRepublish   = toJsRepublish(config);
    auto natsConfig      = toCNatsConfig(config, subjects);
    natsConfig.RePublish = &natsRepublish;

    jsErrCode  jerr;
    const auto status = js_UpdateStream(nullptr, _context, &natsConfig, nullptr, &jerr);

    jsExceptionIfError(status, jerr);
}

bool StreamPrivate::exists(jsCtx* context, const std::string& name)
{
    jsErrCode     jerr;
    jsStreamInfo* natsInfo{ nullptr };

    const auto    status = js_GetStreamInfo(&natsInfo, context, name.c_str(), nullptr, &jerr);
    StreamInfoPtr info(natsInfo, &jsStreamInfo_Destroy);

    if (status == NATS_NOT_FOUND)
        return false;

    jsExceptionIfError(status, jerr);
    return true;
}

StreamInfo StreamPrivate::info(jsCtx* context, const std::string& name)
{
    jsErrCode     jerr;
    jsStreamInfo* natsInfo{ nullptr };

    const auto    status = js_GetStreamInfo(&natsInfo, context, name.c_str(), nullptr, &jerr);
    StreamInfoPtr info(natsInfo, &jsStreamInfo_Destroy);

    jsExceptionIfError(status, jerr);

    return fromCnatsInfo(info.get());
}

std::vector<StreamInfo> StreamPrivate::infos(jsCtx* context)
{
    jsErrCode         jerr;
    jsStreamInfoList* natsInfos{ nullptr };

    const auto        status = js_Streams(&natsInfos, context, nullptr, &jerr);
    StreamInfoListPtr infos(natsInfos, &jsStreamInfoList_Destroy);

    jsExceptionIfError(status, jerr);

    std::vector<StreamInfo> result;
    for (auto i = 0; i < natsInfos->Count; ++i)
        result.push_back(fromCnatsInfo(natsInfos->List[i]));

    return result;
}

std::vector<std::string> StreamPrivate::names(jsCtx* context)
{
    jsErrCode          jerr;
    jsStreamNamesList* natsNames{ nullptr };

    const auto         status = js_StreamNames(&natsNames, context, nullptr, &jerr);
    StreamNamesListPtr names(natsNames, &jsStreamNamesList_Destroy);

    jsExceptionIfError(status, jerr);

    std::vector<std::string> result;
    for (auto i = 0; i < names->Count; ++i)
        result.emplace_back(names->List[i]);

    return result;
}
