#include "SyncSubscription.h"

#include "Message.h"
#include "SyncSubscriptionPrivate.h"

using namespace NatsMq;

SyncSubscription::SyncSubscription(SyncSubscriptionPrivate* impl)
    : _impl(impl)
{
}

SyncSubscription::~SyncSubscription() = default;

SyncSubscription::SyncSubscription(SyncSubscription&&) = default;

SyncSubscription& SyncSubscription::operator=(SyncSubscription&&) = default;

Message SyncSubscription::next() const
{
    return next(0);
}

Message SyncSubscription::next(int64_t timeoutMs) const
{
    return _impl->next(timeoutMs);
}

SubscriptionStatistic SyncSubscription::statistics() const
{
    return _impl->statistics();
}

uint64_t SyncSubscription::messageCount() const
{
    return _impl->messageCount();
}

bool SyncSubscription::isValid() const noexcept
{
    return _impl->isValid();
}

void SyncSubscription::setPendingLimits(int message, int bytes) const
{
    _impl->setPendingLimits({ message, bytes });
}

std::string SyncSubscription::subject() const noexcept
{
    return _impl->subject();
}

int64_t SyncSubscription::id() const noexcept
{
    return _impl->id();
}

void SyncSubscription::setAutoUnsubscribe(int messages) const
{
    _impl->setAutoUnsubscribe(messages);
}

void SyncSubscription::setNoDeliveryDelay() const
{
    _impl->setNoDeliveryDelay();
}

void SyncSubscription::unsubscribe() const
{
    _impl->unsubscribe();
}
