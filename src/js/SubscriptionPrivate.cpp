#include "SubscriptionPrivate.h"

#include "Exceptions.h"
#include "Message.h"
#include "js/MessagePrivate.h"
#include "private/utils.h"

using namespace NatsMq;

namespace
{
    void subscriptionCallback(natsConnection* /*nc*/, natsSubscription* /*sub*/, natsMsg* msg, void* closure)
    {
        auto       cb   = reinterpret_cast<JsSubscriptionCb*>(closure);
        const auto impl = new Js::IncomingMessagePrivate(msg);
        (*cb)(Js::IncomingMessage(impl));
    }

}

Js::SubscriptionPrivate::SubscriptionPrivate(jsCtx* ctx)
    : SubscriptionBaseTemplate(ctx)
{
}

void Js::SubscriptionPrivate::registerListener(const std::string& subject, const SubscriptionOptions& options, JsSubscriptionCb cb)
{
    _cb = std::move(cb);

    auto cnatsSubOptions = Js::toJsCnatsSubOptions(options);

    natsSubscription* natsSub{ nullptr };

    jsErrCode  jerr;
    const auto status = js_Subscribe(&natsSub, _ctx, subject.c_str(), &subscriptionCallback, &_cb, nullptr, &cnatsSubOptions, &jerr);

    jsExceptionIfError(status, jerr);

    _sub.reset(natsSub);
}

void Js::SubscriptionPrivate::drain(int64_t timeoutMs)
{
    if (timeoutMs < 0)
        jsExceptionIfError(natsSubscription_Drain(_sub.get()));
    else
        jsExceptionIfError(natsSubscription_DrainTimeout(_sub.get(), timeoutMs));
}

void Js::SubscriptionPrivate::waitDrain(int64_t timeoutMs) const
{
    jsExceptionIfError(natsSubscription_WaitForDrainCompletion(_sub.get(), timeoutMs));
}
