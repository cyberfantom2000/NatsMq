#include "JsContext.h"

#include "Exceptions.h"
#include "JetStream.h"
#include "JsMessage.h"

NatsMq::Context::Context(natsConnection* connection, jsOptions* options)
    : _context(nullptr, &jsCtx_Destroy)
{
    jsCtx* natsContext = nullptr;

    options->PublishAsync.ErrHandler        = &Context::asyncPublishErrorHandler;
    options->PublishAsync.ErrHandlerClosure = this;

    exceptionIfError(natsConnection_JetStream(&natsContext, connection, options));
    _context.reset(natsContext);
}

jsCtx* NatsMq::Context::rawContext() const
{
    return _context.get();
}

void NatsMq::Context::registerAsyncErrorHandler(JsAsyncErrorCb&& errorHandler)
{
    _errorCb = std::move(errorHandler);
}

void NatsMq::Context::asyncPublishErrorHandler(jsCtx*, jsPubAckErr* pae, void* closure)
{
    const auto context = reinterpret_cast<NatsMq::Context*>(closure);
    if (context && context->_errorCb)
    {
        const auto impl = new NatsMq::JsIncomingMessageImpl(pae->Msg);
        context->_errorCb(NatsMq::JsIncomingMessage(impl), static_cast<Status>(pae->Err), static_cast<JsError>(pae->ErrCode));
    }
}
