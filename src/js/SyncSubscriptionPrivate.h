#pragma once

#include <nats.h>

#include "Entities.h"
#include "js/SubscriptionBaseTemplate.h"

namespace NatsMq
{
    namespace Js
    {
        class SyncSubscriptionPrivate final : public Js::SubscriptionBaseTemplate
        {
        public:
            SyncSubscriptionPrivate(jsCtx* ctx, const std::string& subject, const SubscriptionOptions& options);

            IncomingMessage next(int64_t timeoutMs) const;
        };
    }
};
