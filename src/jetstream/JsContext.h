#pragma once

#include <nats.h>

#include <memory>

#include "Entities.h"

namespace NatsMq
{
    class JetStream;

    class Context
    {
    public:
        using NatsJsContextPtr = std::unique_ptr<jsCtx, decltype(&jsCtx_Destroy)>;

        Context(natsConnection* connection, jsOptions* options);

        jsCtx* rawContext() const;

        void registerAsyncErrorHandler(JsAsyncErrorCb&& errorHandler);

    private:
        static void asyncPublishErrorHandler(jsCtx*, jsPubAckErr* pae, void* closure);

        void errorOccured(jsCtx*, jsPubAckErr* pae, void* closure);

    private:
        JsAsyncErrorCb   _errorCb;
        NatsJsContextPtr _context;
    };
}
