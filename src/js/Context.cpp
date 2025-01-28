#include "Context.h"

#include "Exceptions.h"
#include "Message.h"
#include "private/utils.h"

using namespace NatsMq::Js;

namespace
{
    jsOptions toCnatsJsOptions(const Options& opt)
    {
        jsOptions natsJsOptions;
        jsOptions_Init(&natsJsOptions);
        natsJsOptions.Domain = opt.domain.c_str();
        natsJsOptions.Wait   = opt.timeout;
        natsJsOptions.Prefix = opt.prefix.c_str();

        natsJsOptions.PublishAsync.StallWait  = opt.publishAsync.stallWait;
        natsJsOptions.PublishAsync.MaxPending = opt.publishAsync.maxPending;

        natsJsOptions.Stream.Info.DeletedDetails = opt.stream.info.DeletedDetails;
        natsJsOptions.Stream.Info.SubjectsFilter = opt.stream.info.SubjectsFilter.c_str();

        natsJsOptions.Stream.Purge.Keep     = opt.stream.purge.keep;
        natsJsOptions.Stream.Purge.Sequence = opt.stream.purge.sequence;

        return natsJsOptions;
    }
}

Context::Context(natsConnection* connection, const Options& opt)
    : _context(nullptr, &jsCtx_Destroy)
{
    auto jsOptions = toCnatsJsOptions(opt);

    jsOptions.PublishAsync.ErrHandler        = &Context::asyncPublishErrorHandler;
    jsOptions.PublishAsync.ErrHandlerClosure = this;

    // TODO ack handler

    jsCtx* natsContext{ nullptr };
    jsExceptionIfError(natsConnection_JetStream(&natsContext, connection, &jsOptions));
    _context.reset(natsContext);
}

jsCtx* Context::rawContext() const
{
    return _context.get();
}

void Context::registerAsyncPublishErrorHandler(PublishErrorCb cb)
{
    _errorCb = std::move(cb);
}

void Context::asyncPublishErrorHandler(jsCtx*, jsPubAckErr* pae, void* closure)
{
    const auto context = reinterpret_cast<Context*>(closure);
    if (context && context->_errorCb)
        context->_errorCb(fromCnatsMessage(pae->Msg), static_cast<NatsMq::Status>(pae->Err), static_cast<Js::Status>(pae->ErrCode));
}
