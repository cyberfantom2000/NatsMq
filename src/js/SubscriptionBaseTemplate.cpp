#include "SubscriptionBaseTemplate.h"

using namespace NatsMq;

namespace
{
    using NatsJsConsumerInfoPtr = std::unique_ptr<jsConsumerInfo, decltype(&jsConsumerInfo_Destroy)>;

    Js::SubscriptionMismatch fromCnatsMismathch(const jsConsumerSequenceMismatch& mm)
    {
        Js::SubscriptionMismatch result;
        result.client = mm.ConsumerClient;
        result.server = mm.ConsumerServer;
        result.stream = mm.Stream;
        return result;
    }

    Js::SequnceInfo fromCnatsSequenceInfo(const jsSequenceInfo& i)
    {
        Js::SequnceInfo result;
        result.consumer = i.Consumer;
        result.stream   = i.Stream;
        result.last     = i.Last;
        return result;
    }

    Js::StreamCluster fromCnatsStreamCluster(jsClusterInfo* cl)
    {
        Js::StreamCluster result;

        result.name   = emptyStringIfNull(cl->Name);
        result.leader = emptyStringIfNull(cl->Leader);

        auto replicas = cl->Replicas;
        for (auto i = 0; i < cl->ReplicasLen; ++i)
        {
            auto replica = replicas[i];
            result.replicas.push_back({ replica->Current, replica->Offline, replica->Name, replica->Active, replica->Lag });
        }

        return result;
    }

    Js::Consumer fromCnatsConsumerInfo(jsConsumerInfo* info)
    {
        Js::Consumer result;

        result.stream           = info->Stream;
        result.name             = info->Name;
        result.created          = info->Created;
        result.config           = Js::fromCnatsConsumerConfig(info->Config);
        result.delivered        = fromCnatsSequenceInfo(info->Delivered);
        result.ackFloor         = fromCnatsSequenceInfo(info->AckFloor);
        result.ackPendingCount  = info->NumAckPending;
        result.redeliveredCount = info->NumRedelivered;
        result.waitingCount     = info->NumWaiting;
        result.pendingCount     = info->NumPending;
        if (info->Cluster)
            result.cluster = fromCnatsStreamCluster(info->Cluster);

        return result;
    }
}

Js::SubscriptionBaseTemplate::~SubscriptionBaseTemplate()
{
    natsSubscription_Unsubscribe(_sub.get());
}

Js::SubscriptionMismatch Js::SubscriptionBaseTemplate::mismatch() const
{
    jsConsumerSequenceMismatch mm;

    const auto status = natsSubscription_GetSequenceMismatch(&mm, _sub.get());

    jsExceptionIfError(status);

    return fromCnatsMismathch(mm);
}

Js::Consumer Js::SubscriptionBaseTemplate::consumerInfo() const
{
    jsConsumerInfo* info{ nullptr };

    jsErrCode  jerr;
    const auto status = natsSubscription_GetConsumerInfo(&info, _sub.get(), nullptr, &jerr);

    jsExceptionIfError(status, jerr);

    NatsJsConsumerInfoPtr ptr(info, &jsConsumerInfo_Destroy);

    return fromCnatsConsumerInfo(info);
}
