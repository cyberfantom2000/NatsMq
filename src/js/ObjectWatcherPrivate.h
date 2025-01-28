#pragma once

#include <nats.h>

#include "Entities.h"
#include "js/SubscriptionPrivate.h"

namespace NatsMq
{
    namespace Js
    {
        class ObjectWatcherPrivate
        {
        public:
            ObjectWatcherPrivate(jsCtx* ctx, const std::string& stream, const std::string& subject);

            void stop() const;

            void updated(ObjectWatchCb cb);

            void removed(ObjectWatchCb cb);

        private:
            SubscriptionPrivate _sub;
            ObjectWatchCb       _updated;
            ObjectWatchCb       _removed;
        };
    }
}
