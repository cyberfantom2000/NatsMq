#include "SyncSubscriptionPrivate.h"

#include "Message.h"
#include "js/MessagePrivate.h"
using namespace NatsMq;

Js::SyncSubscriptionPrivate::SyncSubscriptionPrivate(jsCtx* ctx, const std::string& subj, const SubscriptionOptions& options)
    : SubscriptionBaseTemplate(ctx, &js_SubscribeSync, options, subj.c_str(), nullptr)
{
}

Js::IncomingMessage Js::SyncSubscriptionPrivate::next(int64_t timeoutMs) const
{
    natsMsg* msg{ nullptr };
    jsExceptionIfError(natsSubscription_NextMsg(&msg, _sub.get(), timeoutMs));
    return new Js::IncomingMessagePrivate(msg);
}
