#include "Subscription.h"

#include "js/SubscriptionPrivate.h"

using namespace NatsMq;

Js::Subscription::Subscription(Js::SubscriptionPrivate* impl)
    : _impl(impl)
{
}

Js::Subscription& Js::Subscription::operator=(Subscription&&) = default;

Js::Subscription::~Subscription() = default;

Js::Subscription::Subscription(Js::Subscription&&) = default;

SubscriptionStatistic Js::Subscription::statistics() const
{
    return _impl->statistics();
}

bool Js::Subscription::isValid() const noexcept
{
    return _impl->isValid();
}

void Js::Subscription::drain() const
{
    drain(-1);
}

void Js::Subscription::drain(int64_t timeout) const
{
    _impl->drain(timeout);
}

uint64_t Js::Subscription::messageCount() const
{
    return _impl->messageCount();
}

void Js::Subscription::setPendingLimits(int message, int bytes) const
{
    _impl->setPendingLimits({ message, bytes });
}

std::string Js::Subscription::subject() const noexcept
{
    return _impl->subject();
}

int64_t Js::Subscription::id() const noexcept
{
    return _impl->id();
}

void Js::Subscription::setAutoUnsubscribe(int messages) const
{
    _impl->setAutoUnsubscribe(messages);
}

void Js::Subscription::setNoDeliveryDelay() const
{
    _impl->setNoDeliveryDelay();
}

void Js::Subscription::unsubscribe() const
{
    _impl->unsubscribe();
}

Js::SubscriptionMismatch Js::Subscription::mismatch() const
{
    return _impl->mismatch();
}

Js::Consumer Js::Subscription::consumerInfo() const
{
    return _impl->consumerInfo();
}
