#pragma once

#include <nats.h>

#include <memory>

#include "Entities.h"
#include "private/defines.h"

namespace NatsMq
{
    namespace Js
    {
        class Context
        {
        public:
            Context(natsConnection* connection, const Options& opt);

            jsCtx* rawContext() const;

            void registerAsyncPublishErrorHandler(PublishErrorCb errorHandler);

        private:
            static void asyncPublishErrorHandler(jsCtx*, jsPubAckErr* pae, void* closure);

        private:
            PublishErrorCb   _errorCb;
            NatsJsContextPtr _context;
        };
    }
}
