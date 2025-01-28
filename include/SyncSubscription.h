#pragma once

#include <memory>

#include "Entities.h"
#include "Export.h"

namespace NatsMq
{
    class SyncSubscriptionPrivate;

    class NATSMQ_EXPORT SyncSubscription
    {
    public:
        SyncSubscription(SyncSubscriptionPrivate*);

        ~SyncSubscription();

        SyncSubscription(SyncSubscription&&);

        SyncSubscription& operator=(SyncSubscription&&);

        //! Get next message. if there are no messages an exception will be thrown.
        Message next() const;

        //! Similar as next(), but waits message during the timeout
        Message next(int64_t timeoutMs) const;

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

        // TODO проверить нужен ли в синхронной подписке этот метод
        //! Call unsubscribe after 'messages' have been received
        void setAutoUnsubscribe(int messages) const;

        //! By default, messages that arrive are not immediately delivered. This generally improves performance.
        //! However, in case of request-reply, this delay has a negative impact.
        void setNoDeliveryDelay() const;

        //! Removes interest on the subject
        void unsubscribe() const;

    private:
        std::unique_ptr<SyncSubscriptionPrivate> _impl;
    };

    namespace Js
    {
        class SyncSubscriptionPrivate;

        class NATSMQ_EXPORT SyncSubscription
        {
        public:
            SyncSubscription(Js::SyncSubscriptionPrivate* impl);

            ~SyncSubscription();

            SyncSubscription(SyncSubscription&&);

            SyncSubscription& operator=(SyncSubscription&&);

            //! Get next message. if there are no messages an exception will be thrown.
            IncomingMessage next() const;

            //! Similar as next(), but waits message during the timeout
            IncomingMessage next(int64_t timeoutMs) const;

            //! Fetches up to batch messages from the server, waiting up to timeout milliseconds.
            //! No more thant batch messages will be returned, however, it can be less.
            std::vector<IncomingMessage> fetch(int batch, int64_t timeoutMs = 2000) const;

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
            std::unique_ptr<Js::SyncSubscriptionPrivate> _impl;
        };
    }
}
