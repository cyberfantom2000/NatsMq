#pragma once

#include "Entities.h"
#include "Message.h"

namespace NatsMq
{
    class SubscriptionImpl;

    class Subscription
    {
    public:
        Subscription(SubscriptionImpl*);

        ~Subscription();

        SubscriptionStatistic statistics() const;

        //! Called all times for a new message
        int registerListener(const SubscriptionCb& cb);

        int registerListener(SubscriptionCb&& cb);

        void unregisterListener(int idx);

    private:
        std::unique_ptr<SubscriptionImpl> _impl;
    };

    class JsSubscriptionImpl;

    class JsSubscription
    {
    public:
        JsSubscription() = default;

        JsSubscription(JsSubscriptionImpl*);

        ~JsSubscription();

        //! Called all times for a new message
        int registerListener(const JsSubscriptionCb& cb);

        int registerListener(JsSubscriptionCb&& cb);

        void unregisterListener(int idx);

    private:
        std::unique_ptr<JsSubscriptionImpl> _impl;
    };

    class JsPullSubscriptionImpl;

    class JsPullSubscription
    {
    public:
        JsPullSubscription() = default;

        JsPullSubscription(JsPullSubscriptionImpl*);

        ~JsPullSubscription();

        JsPullSubscription(JsPullSubscription&&);

        JsPullSubscription& operator=(JsPullSubscription&&);

        //! Fetch new messages from server
        std::vector<JsIncomingMessage> fetch(int batch = 1, uint64_t timeoutMs = 5000) const;

    private:
        std::unique_ptr<JsPullSubscriptionImpl> _impl;
    };
}
