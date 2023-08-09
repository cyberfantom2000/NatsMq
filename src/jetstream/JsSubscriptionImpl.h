#pragma once

#include <nats.h>

#include <memory>

#include "Entities.h"

namespace NatsMq
{
    struct JsIncomingMessage;

    using NatsSubscriptionPtr = std::unique_ptr<natsSubscription, decltype(&natsSubscription_Destroy)>;

    class JsSubscriptionImpl
    {
    public:
        static JsSubscriptionImpl* create(jsCtx* context, const std::string& stream, const std::string& subject, const std::string& consumer);

        JsSubscriptionImpl();

        JsSubscriptionImpl(natsSubscription*);

        int registerListener(JsSubscriptionCb&& cb);

        void unregisterListener(int idx);

    private:
        static void subscriptionCallback(natsConnection* /*nc*/, natsSubscription* /*sub*/, natsMsg* msg, void* closure);

        void dataReady(const NatsMq::JsIncomingMessage& msg);

    private:
        std::vector<JsSubscriptionCb> _callbacks;
        NatsSubscriptionPtr           _sub;
    };

    class JsPullSubscriptionImpl
    {
    public:
        static JsPullSubscriptionImpl* create(jsCtx* context, const std::string& stream, const std::string& subject, const std::string& consumer);

        JsPullSubscriptionImpl(natsSubscription*);

        std::vector<JsIncomingMessage> fetch(int batch, uint64_t timeoutMs) const;

    private:
        NatsSubscriptionPtr _sub;
    };
}
