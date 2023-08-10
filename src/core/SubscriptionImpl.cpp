#include "SubscriptionImpl.h"

#include "Exceptions.h"
#include "MessageImpl.h"
#include "Subscription.h"

namespace
{
    using PrivateSubPtr = std::unique_ptr<NatsMq::SubscriptionImpl>;
}

NatsMq::Subscription::Subscription(SubscriptionImpl* sub)
    : _impl(sub)
{
}

NatsMq::Subscription::Subscription(Subscription&&) = default;

NatsMq::Subscription& NatsMq::Subscription::operator=(Subscription&&) = default;

NatsMq::SubscriptionStatistic NatsMq::Subscription::statistics() const
{
    return _impl->statistics();
}

int NatsMq::Subscription::registerListener(const SubscriptionCb& cb)
{
    return registerListener(SubscriptionCb(cb));
}

int NatsMq::Subscription::registerListener(SubscriptionCb&& cb)
{
    return _impl->registerListener(std::move(cb));
}

void NatsMq::Subscription::unregisterListener(int idx)
{
    _impl->unregisterListener(idx);
}

NatsMq::Subscription::~Subscription() = default;

NatsMq::SubscriptionImpl* NatsMq::SubscriptionImpl::create(natsConnection* connection, const std::string& subject)
{
    natsSubscription* natsSub{ nullptr };

    auto subscription = std::make_unique<NatsMq::SubscriptionImpl>(natsSub);
    exceptionIfError(natsConnection_Subscribe(&natsSub, connection, subject.c_str(), &subscriptionCallback, subscription.get()));

    subscription->_sub.reset(natsSub);

    return subscription.release();
}

NatsMq::SubscriptionImpl* NatsMq::SubscriptionImpl::create(natsConnection* connection, const std::string& subject, const std::string& queueGroup)
{
    natsSubscription* natsSub{ nullptr };

    auto subscription = std::make_unique<NatsMq::SubscriptionImpl>(natsSub);
    exceptionIfError(natsConnection_QueueSubscribe(&natsSub, connection, subject.c_str(), queueGroup.c_str(), &subscriptionCallback, subscription.get()));

    subscription->_sub.reset(natsSub);

    return subscription.release();
}

NatsMq::SubscriptionImpl::SubscriptionImpl(natsSubscription* sub)
    : _sub(sub, &natsSubscription_Destroy)
{
}

NatsMq::SubscriptionStatistic NatsMq::SubscriptionImpl::statistics() const
{
    SubscriptionStatistic stats;
    exceptionIfError(natsSubscription_GetStats(_sub.get(), &stats.pendingMsgs, &stats.pendingBytes,
                                               &stats.maxPendingMsgs, &stats.maxPendingBytes,
                                               &stats.deliveredMsgs, &stats.droppedMsgs));
    return stats;
}

int NatsMq::SubscriptionImpl::registerListener(SubscriptionCb&& cb)
{
    _callbacks.push_back(std::move(cb));
    return static_cast<int>(_callbacks.size()) - 1;
}

void NatsMq::SubscriptionImpl::unregisterListener(int idx)
{
    _callbacks.erase(_callbacks.begin() + idx);
}

void NatsMq::SubscriptionImpl::subscriptionCallback(natsConnection*, natsSubscription*, natsMsg* msg, void* closure)
{
    const auto sub = reinterpret_cast<NatsMq::SubscriptionImpl*>(closure);

    NatsMq::IncomingMessageImpl impl(msg);

    sub->dataReady(NatsMq::IncomingMessage(&impl));
}

void NatsMq::SubscriptionImpl::dataReady(const IncomingMessage& msg)
{
    for (auto&& cb : _callbacks)
        cb(msg);
}
