#pragma once

#include <nats.h>

#include <string>
#include <vector>

#include "Entities.h"
#include "private/defines.h"

namespace NatsMq
{
    struct Message;

    NatsMq::NatsMsgPtr createCnatsMessage(const Message& msg);

    NatsMq::Message fromCnatsMessage(natsMsg* msg);

    bool makePing(natsConnection*, int timeout);

    void configurePoolSize(int poolSize);

    std::string emptyStringIfNull(const char* s);

    std::vector<const char*> createArrayPointersToElements(const std::vector<std::string>& elements);

    natsMetadata toNatsMetadata(std::vector<const char*>& data);

    namespace Js
    {
        struct SubscriptionOptions;
        struct ConsumerConfig;

        jsSubOptions toJsCnatsSubOptions(const SubscriptionOptions& options);

        jsConsumerConfig toJsConsumerConfig(const ConsumerConfig& config);

        ConsumerConfig fromCnatsConsumerConfig(jsConsumerConfig* cfg);

        std::vector<uint8_t> serializeObjectMeta(const Js::ObjectInfo& info);

        Js::ObjectInfo deserializeObjectMeta(const NatsMq::Message& msg);
    }

}

namespace Utils
{
    std::string uuid();
}
