#include "JsSubscriptionImpl.h"

#include "Exceptions.h"
#include "JsMessage.h"
#include "Subscription.h"

namespace
{
    jsSubOptions createJsSubOptions(const std::string& stream, const std::string& consumer)
    {
        jsSubOptions options;
        jsSubOptions_Init(&options);
        options.Stream    = stream.c_str();
        options.Consumer  = consumer.c_str();
        options.ManualAck = true;
        return options;
    }
}

NatsMq::JsSubscription::JsSubscription(JsSubscriptionImpl* sub)
    : _impl(sub)
{
}

int NatsMq::JsSubscription::registerListener(const JsSubscriptionCb& cb)
{
    return registerListener(JsSubscriptionCb(cb));
}

int NatsMq::JsSubscription::registerListener(JsSubscriptionCb&& cb)
{
    return _impl->registerListener(std::move(cb));
}

void NatsMq::JsSubscription::unregisterListener(int idx)
{
    _impl->unregisterListener(idx);
}

NatsMq::JsSubscription::~JsSubscription() = default;

NatsMq::JsPullSubscription::JsPullSubscription(JsPullSubscriptionImpl* impl)
    : _impl(impl)
{
}

NatsMq::JsPullSubscription::~JsPullSubscription() = default;

NatsMq::JsPullSubscription::JsPullSubscription(JsPullSubscription&&) = default;

NatsMq::JsPullSubscription& NatsMq::JsPullSubscription::operator=(JsPullSubscription&&) = default;

std::vector<NatsMq::JsIncomingMessage> NatsMq::JsPullSubscription::fetch(int batch, uint64_t timeoutMs) const
{
    return _impl->fetch(batch, timeoutMs);
}

NatsMq::JsSubscriptionImpl* NatsMq::JsSubscriptionImpl::create(jsCtx* context, const std::string& stream, const std::string& subject, const std::string& consumer)
{
    auto options = createJsSubOptions(stream, consumer);

    natsSubscription* natsSub{ nullptr };
    auto              subscription = std::make_unique<NatsMq::JsSubscriptionImpl>();

    jsErrCode  jsErr;
    const auto status = js_Subscribe(&natsSub, context, subject.c_str(), &subscriptionCallback, subscription.get(), nullptr, &options, &jsErr);

    subscription->_sub.reset(natsSub);

    jsExceptionIfError(status, jsErr);
    return subscription.release();
}

NatsMq::JsSubscriptionImpl::JsSubscriptionImpl()
    : _sub(nullptr, &natsSubscription_Destroy)
{
}

NatsMq::JsSubscriptionImpl::JsSubscriptionImpl(natsSubscription* sub)
    : _sub(sub, &natsSubscription_Destroy)
{
}

int NatsMq::JsSubscriptionImpl::registerListener(JsSubscriptionCb&& cb)
{
    _callbacks.push_back(std::move(cb));
    return static_cast<int>(_callbacks.size()) - 1;
}

void NatsMq::JsSubscriptionImpl::unregisterListener(int idx)
{
    _callbacks.erase(_callbacks.begin() + idx);
}

void NatsMq::JsSubscriptionImpl::subscriptionCallback(natsConnection*, natsSubscription*, natsMsg* msg, void* closure)
{
    const auto sub  = reinterpret_cast<NatsMq::JsSubscriptionImpl*>(closure);
    const auto impl = new NatsMq::JsIncomingMessageImpl(msg);
    sub->dataReady(NatsMq::JsIncomingMessage(impl));
}

void NatsMq::JsSubscriptionImpl::dataReady(const JsIncomingMessage& msg)
{
    for (auto&& cb : _callbacks)
        cb(msg);
}

NatsMq::JsPullSubscriptionImpl* NatsMq::JsPullSubscriptionImpl::create(jsCtx* context, const std::string& stream, const std::string& subject, const std::string& durable)
{
    auto options = createJsSubOptions(stream, durable);

    natsSubscription* natsSub{ nullptr };

    jsErrCode  jsErr;
    const auto status = js_PullSubscribe(&natsSub, context, subject.c_str(), nullptr, nullptr, &options, &jsErr);

    auto subscription = std::make_unique<NatsMq::JsPullSubscriptionImpl>(natsSub);

    jsExceptionIfError(status, jsErr);
    return subscription.release();
}

NatsMq::JsPullSubscriptionImpl::JsPullSubscriptionImpl(natsSubscription* sub)
    : _sub(sub, &natsSubscription_Destroy)
{
}

std::vector<NatsMq::JsIncomingMessage> NatsMq::JsPullSubscriptionImpl::fetch(int batch, uint64_t timeoutMs) const
{
    natsMsgList natsMsgs{ nullptr, 0 };
    jsErrCode   jsErr;

    const auto status = natsSubscription_Fetch(&natsMsgs, _sub.get(), batch, timeoutMs, &jsErr);
    jsExceptionIfError(status, jsErr);

    std::vector<NatsMq::JsIncomingMessage> msgs;
    for (auto i = 0; i < natsMsgs.Count; ++i)
    {
        const auto impl = new NatsMq::JsIncomingMessageImpl(natsMsgs.Msgs[i]);
        msgs.push_back(NatsMq::JsIncomingMessage(impl));
        natsMsgs.Msgs[i] = nullptr;
    }

    natsMsgList_Destroy(&natsMsgs);
    return msgs;
}
