#include "PullSubscriptionPrivate.h"

#include "Message.h"
#include "js/MessagePrivate.h"
using namespace NatsMq;

namespace
{
    std::vector<Js::IncomingMessage> fromCnatsMessageList(const natsMsgList& list)
    {
        std::vector<Js::IncomingMessage> result;
        for (auto i = 0; i < list.Count; ++i)
            result.emplace_back(new Js::IncomingMessagePrivate(list.Msgs[i]));

        return result;
    }
}

Js::PullSubscriptionPrivate::PullSubscriptionPrivate(jsCtx* ctx, const std::string& subject, const SubscriptionOptions& options)
    : SubscriptionBaseTemplate(ctx, &js_PullSubscribe, options, subject.c_str(), nullptr, nullptr)
{
}

std::vector<Js::IncomingMessage> Js::PullSubscriptionPrivate::fetch(int batch, int64_t timeoutMs) const
{
    natsMsgList msgs;

    jsErrCode  jerr;
    const auto status = natsSubscription_Fetch(&msgs, _sub.get(), batch, timeoutMs, &jerr);

    jsExceptionIfError(status, jerr);

    return fromCnatsMessageList(msgs);
}
