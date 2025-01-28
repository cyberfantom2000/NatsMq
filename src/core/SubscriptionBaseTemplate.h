#pragma once

#include "Exceptions.h"
#include "private/SubscriptionBasePrivate.h"
#include "private/defines.h"

namespace NatsMq
{
    class SubscriptionBaseTemplate : public SubscriptionBasePrivate<NatsSubscriptionPtr, decltype(natsSubscription_Destroy)>
    {
    public:
        template <typename Creator, typename... Args>
        SubscriptionBaseTemplate(Creator creator, Args... args)
            : SubscriptionBasePrivate(nullptr, &natsSubscription_Destroy)
        {
            natsSubscription* natsSub{ nullptr };
            exceptionIfError(creator(&natsSub, args...));
            _sub.reset(natsSub);
        }
    };
};
