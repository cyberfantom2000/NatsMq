#pragma once

#include <memory>

#include "Entities.h"
#include "Export.h"

namespace NatsMq
{
    class SubscriptionPrivate;

    class NATSMQ_EXPORT Subscription
    {
    public:
        Subscription(SubscriptionPrivate*);

        ~Subscription();

        Subscription(Subscription&&);

        Subscription& operator=(Subscription&&);

        //! Get subscription statistics
        SubscriptionStatistic statistics() const;

        //! Check subscription status
        bool isValid() const noexcept;

        //! Drain will remove interest but continue to invoke callbacks until all messages have been processed.
        void drain() const;

        //! Similar as drain(), but messages that were not processed within the timeout will be lost
        void drain(int64_t timeout) const;

        //! Get message count
        uint64_t messageCount() const;

        //! Specifies the maximum number and size of incoming messages that can be buffered in the library
        //! for this subscription, before new incoming messages are dropped and NATS_SLOW_CONSUMER status is reported
        void setPendingLimits(int message, int bytes) const;

        //! Get subscription subject
        std::string subject() const noexcept;

        //! Get subscription id
        int64_t id() const noexcept;

        //! Call unsubscribe after 'messages' have been received
        void setAutoUnsubscribe(int messages) const;

        //! By default, messages that arrive are not immediately delivered. This generally improves performance.
        //! However, in case of request-reply, this delay has a negative impact.
        void setNoDeliveryDelay() const;

        //! Removes interest on the subject
        void unsubscribe() const;

    private:
        std::unique_ptr<SubscriptionPrivate> _impl;
    };

    namespace Js
    {
        class SubscriptionPrivate;

        class NATSMQ_EXPORT Subscription
        {
        public:
            Subscription(Js::SubscriptionPrivate*);

            ~Subscription();

            Subscription(Subscription&&);

            Subscription& operator=(Subscription&&);

            //! Get subscription statistics
            SubscriptionStatistic statistics() const;

            //! Check subscription status
            bool isValid() const noexcept;

            //! Drain will remove interest but continue to invoke callbacks until all messages have been processed.
            void drain() const;

            //! Similar as drain(), but messages that were not processed within the timeout will be lost
            void drain(int64_t timeout) const;

            //! Returns the number of queued messages in the client for this subscription.
            uint64_t messageCount() const;

            //! Specifies the maximum number and size of incoming messages that can be buffered in the library
            //! for this subscription, before new incoming messages are dropped and NATS_SLOW_CONSUMER status is reported
            void setPendingLimits(int message, int bytes) const;

            //! Get subscription subject
            std::string subject() const noexcept;

            //! Get subscription id
            int64_t id() const noexcept;

            //! Call unsubscribe after 'messages' have been received
            void setAutoUnsubscribe(int messages) const;

            //! By default, messages that arrive are not immediately delivered. This generally improves performance.
            //! However, in case of request-reply, this delay has a negative impact.
            void setNoDeliveryDelay() const;

            //! Removes interest on the subject
            void unsubscribe() const;

            //! Mismatch between the server and client's view of the state of the consumer.
            //! see more http://nats-io.github.io/nats.c/group__js_sub_group.html#ga0daed9f0d2d00cb14314ecc180a6fa84
            SubscriptionMismatch mismatch() const;

            //! Consumer information associated with this subscription.
            Consumer consumerInfo() const;

        private:
            std::unique_ptr<Js::SubscriptionPrivate> _impl;
        };
    }
}
