#pragma once

#include <nats.h>

#include <future>

#include "Message.h"

namespace NatsMq
{
    class JsPublisher
    {
    public:
        JsPublisher(jsCtx* context);

        JsPublishAck publish(const Message& msg, const JsPublishOptions& options) const;

        JsPublishAck asyncPublish(const Message& msg, const JsPublishOptions& options) const;

        void waitPublishCompleted(int64_t timeout) const;

        std::vector<Message> getAsyncPendingMessages() const;

    private:
        JsPublishAck makePublish(natsMsg* msg, jsPubOptions* options) const;

        JsPublishAck makeAsyncPublish(const Message& msg, jsPubOptions* options) const;

    private:
        jsCtx* _context;
    };
}
