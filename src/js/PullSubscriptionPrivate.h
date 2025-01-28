#pragma once

#include <nats.h>

#include "Entities.h"
#include "js/SubscriptionBaseTemplate.h"

namespace NatsMq
{

    namespace Js
    {
        class PullSubscriptionPrivate final : public Js::SubscriptionBaseTemplate
        {
        public:
            PullSubscriptionPrivate(jsCtx* ctx, const std::string& subject, const SubscriptionOptions& options);

            std::vector<IncomingMessage> fetch(int batch, int64_t timeoutMs) const;
        };

    }

}
