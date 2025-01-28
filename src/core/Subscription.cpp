
#include "Subscription.h"

#include "SubscriptionPrivate.h"

using namespace NatsMq;

Subscription::Subscription(SubscriptionPrivate* impl)
    : _impl(impl)
{
}

Subscription::~Subscription() = default;

Subscription::Subscription(Subscription&&) = default;

Subscription& Subscription::operator=(Subscription&&) = default;

SubscriptionStatistic Subscription::statistics() const
{
    return _impl->statistics();
}

bool Subscription::isValid() const noexcept
{
    return _impl->isValid();
}

void Subscription::drain() const
{
    drain(-1);
}

void Subscription::drain(int64_t timeout) const
{
    _impl->drain(timeout);
}

uint64_t Subscription::messageCount() const
{
    return _impl->messageCount();
}

void Subscription::setPendingLimits(int message, int bytes) const
{
    _impl->setPendingLimits({ message, bytes });
}

std::string Subscription::subject() const noexcept
{
    return _impl->subject();
}

int64_t Subscription::id() const noexcept
{
    return _impl->id();
}

void Subscription::setAutoUnsubscribe(int messages) const
{
    _impl->setAutoUnsubscribe(messages);
}

void Subscription::setNoDeliveryDelay() const
{
    _impl->setNoDeliveryDelay();
}

void Subscription::unsubscribe() const
{
    _impl->unsubscribe();
}
