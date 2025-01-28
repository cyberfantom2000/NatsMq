#pragma once

#include <nats.h>

#include <memory>

#include "Entities.h"
#include "core/SubscriptionBaseTemplate.h"

namespace NatsMq
{
    class SyncSubscriptionPrivate final : public SubscriptionBaseTemplate
    {
    public:
        SyncSubscriptionPrivate(natsConnection* connection, const std::string& subject);

        SyncSubscriptionPrivate(natsConnection* connection, const std::string& subject, const std::string& queue);

        Message next(int64_t timeoutMs) const;
    };
}
