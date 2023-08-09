#pragma once

#include <nats.h>

#include <memory>

#include "Entities.h"

namespace NatsMq
{
    struct IncomingMessage;

    struct SubscriptionStatistic;

    using NatsSubscriptionPtr = std::unique_ptr<natsSubscription, decltype(&natsSubscription_Destroy)>;

    class SubscriptionImpl
    {
    public:
        static SubscriptionImpl* create(natsConnection* connection, const std::string& subject);

        static SubscriptionImpl* create(natsConnection* connection, const std::string& subject, const std::string& queueGroup);

        SubscriptionImpl(natsSubscription*);

        SubscriptionStatistic statistics() const;

        int registerListener(SubscriptionCb&& cb);

        void unregisterListener(int idx);

    private:
        static void subscriptionCallback(natsConnection* nc, natsSubscription* sub, natsMsg* msg, void* closure);

        void dataReady(const NatsMq::IncomingMessage& msg);

    private:
        std::vector<SubscriptionCb> _callbacks;
        NatsSubscriptionPtr         _sub;
    };

}
