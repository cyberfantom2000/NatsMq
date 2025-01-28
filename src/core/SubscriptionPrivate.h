#pragma once

#include <nats.h>

#include <memory>

#include "Entities.h"
#include "core/SubscriptionBaseTemplate.h"

namespace NatsMq
{
    class SubscriptionPrivate final : public SubscriptionBaseTemplate
    {
    public:
        SubscriptionPrivate(natsConnection* connection, const std::string& subject);

        SubscriptionPrivate(natsConnection* connection, const std::string& subject, int64_t timeoutMs);

        SubscriptionPrivate(natsConnection* connection, const std::string& subject, const std::string& queue);

        SubscriptionPrivate(natsConnection* connection, const std::string& subject, const std::string& queue, int64_t timeoutMs);

        void registerListener(SubscriptionCb cb);

        void drain(int64_t timeout) const;

        void waitDrain(int64_t timeoutMs = 0) const;

        Status drainStatus() const;

    private:
        static void subscriptionCallback(natsConnection* nc, natsSubscription* sub, natsMsg* msg, void* closure);

        void dataReady(Message msg);

    private:
        SubscriptionCb _cb;
    };
}
