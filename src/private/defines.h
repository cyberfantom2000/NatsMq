#pragma once

#include <nats.h>

#include <memory>

namespace NatsMq
{
    using NatsSubscriptionPtr = std::unique_ptr<natsSubscription, decltype(&natsSubscription_Destroy)>;
    using NatsMsgPtr          = std::unique_ptr<natsMsg, decltype(&natsMsg_Destroy)>;

    // starting with v3.4.0, if the user calls only natsSubscription_Destroy (to free memory), the JetStream consumer will no longer be deleted.
    inline void jsSubDeleter(natsSubscription* sub)
    {
        natsSubscription_Unsubscribe(sub);
        natsSubscription_Destroy(sub);
    };

    using NatsJsSubscriptionPtr = std::unique_ptr<natsSubscription, decltype(&jsSubDeleter)>;

    namespace Js
    {
        using NatsJsContextPtr = std::unique_ptr<jsCtx, decltype(&jsCtx_Destroy)>;

    }
}
