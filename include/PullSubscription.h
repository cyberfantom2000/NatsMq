#pragma once

#include <Message.h>

#include "Export.h"

namespace NatsMq
{
    namespace Js
    {
        class PullSubscriptionPrivate;

        class NATSMQ_EXPORT PullSubscription
        {
        public:
            PullSubscription(PullSubscriptionPrivate* impl);

            ~PullSubscription();

            PullSubscription(PullSubscription&&);

            PullSubscription& operator=(PullSubscription&&);

            //! Fetches up to batch messages from the server, waiting up to timeout milliseconds.
            //! No more thant batch messages will be returned, however, it can be less.
            std::vector<IncomingMessage> fetch(int batch, int64_t timeoutMs = 2000);

            //! Get subscription statistics
            SubscriptionStatistic statistics() const;

            //! Returns the number of queued messages in the client for this subscription.
            uint64_t messageCount() const;

            //! Check subscription status
            bool isValid() const noexcept;

            //! Specifies the maximum number and size of incoming messages that can be buffered in the library
            //! for this subscription, before new incoming messages are dropped and NATS_SLOW_CONSUMER status is reported
            void setPendingLimits(int message, int bytes) const;

            //! Get subscription subject
            std::string subject() const noexcept;

            //! Get subscription id
            int64_t id() const noexcept;

            //! Removes interest on the subject
            void unsubscribe() const;

        private:
            std::unique_ptr<PullSubscriptionPrivate> _impl;
        };
    }
}
