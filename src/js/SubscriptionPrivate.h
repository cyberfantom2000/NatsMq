#pragma once

#include <nats.h>

#include "Entities.h"
#include "js/SubscriptionBaseTemplate.h"

namespace NatsMq
{
    namespace Js
    {
        class SubscriptionPrivate final : public Js::SubscriptionBaseTemplate
        {
        public:
            SubscriptionPrivate(jsCtx* ctx);

            void registerListener(const std::string& subject, const SubscriptionOptions& options, JsSubscriptionCb cb);

            void drain(int64_t timeoutMs);

            void waitDrain(int64_t timeoutMs = 0) const;

        private:
            JsSubscriptionCb _cb;
        };
    }
}
