#pragma once

#include <nats.h>

#include "Exceptions.h"

namespace NatsMq
{
    template <typename SubPtr, typename Deleter>
    class SubscriptionBasePrivate
    {
    public:
        struct Pending
        {
            int messages;
            int bytes;
        };

        SubscriptionBasePrivate(natsSubscription* sub, Deleter* deleter)
            : _sub(sub, deleter)
        {
        }

        virtual ~SubscriptionBasePrivate() = default;

        SubscriptionBasePrivate(SubscriptionBasePrivate&&) = default;

        SubscriptionBasePrivate& operator=(SubscriptionBasePrivate&&) = default;

        bool isValid() const noexcept
        {
            return natsSubscription_IsValid(_sub.get());
        }

        SubscriptionStatistic statistics() const
        {
            SubscriptionStatistic stats;
            exceptionIfError(natsSubscription_GetStats(_sub.get(), &stats.pendingMsgs, &stats.pendingBytes,
                                                       &stats.maxPendingMsgs, &stats.maxPendingBytes,
                                                       &stats.deliveredMsgs, &stats.droppedMsgs));
            return stats;
        }

        uint64_t messageCount() const
        {
            uint64_t count;
            exceptionIfError(natsSubscription_QueuedMsgs(_sub.get(), &count));
            return count;
        }

        void setPendingLimits(Pending limits) const
        {
            exceptionIfError(natsSubscription_SetPendingLimits(_sub.get(), limits.messages, limits.bytes));
        }

        std::string subject() const noexcept
        {
            return natsSubscription_GetSubject(_sub.get());
        }

        int64_t id() const noexcept
        {
            return natsSubscription_GetID(_sub.get());
        }

        void setAutoUnsubscribe(int messages) const
        {
            exceptionIfError(natsSubscription_AutoUnsubscribe(_sub.get(), messages));
        }

        void setNoDeliveryDelay() const
        {
            exceptionIfError(natsSubscription_NoDeliveryDelay(_sub.get()));
        }

        void unsubscribe() const
        {
            exceptionIfError(natsSubscription_Unsubscribe(_sub.get()));
        }

    protected:
        SubPtr _sub;
    };

}
