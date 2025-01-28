#pragma once
#include <nats.h>

#include "Entities.h"

namespace NatsMq
{
    namespace Js
    {
        class Publisher
        {
        public:
            Publisher(jsCtx* context);

            Js::PublishAck publish(Message msg, Js::PublishOptions options) const;

            void apublish(Message msg, Js::PublishOptions options) const;

            void waitAsyncPublishComplete(int64_t timeoutMs) const;

            std::vector<Message> asyncPendingMessages() const;

        private:
            Js::PublishAck makePublish(natsMsg* msg, jsPubOptions* options) const;

            void makeAsyncPublish(natsMsg* msg, jsPubOptions* options) const;

        private:
            jsCtx* _context;
        };
    }
}
