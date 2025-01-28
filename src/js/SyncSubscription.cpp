#include "SyncSubscription.h"

#include "Message.h"
#include "js/SyncSubscriptionPrivate.h"

using namespace NatsMq;

Js::SyncSubscription::SyncSubscription(Js::SyncSubscriptionPrivate* impl)
    : _impl(impl)
{
}

Js::SyncSubscription::~SyncSubscription() = default;

Js::SyncSubscription::SyncSubscription(SyncSubscription&&) = default;

Js::SyncSubscription& Js::SyncSubscription::operator=(SyncSubscription&&) = default;

Js::IncomingMessage Js::SyncSubscription::next() const
{
    return _impl->next(0);
}

Js::IncomingMessage Js::SyncSubscription::next(int64_t timeoutMs) const
{
    return _impl->next(timeoutMs);
}

SubscriptionStatistic Js::SyncSubscription::statistics() const
{
    return _impl->statistics();
}

uint64_t Js::SyncSubscription::messageCount() const
{
    return _impl->messageCount();
}

bool Js::SyncSubscription::isValid() const noexcept
{
    return _impl->isValid();
}

void Js::SyncSubscription::setPendingLimits(int message, int bytes) const
{
    _impl->setPendingLimits({ message, bytes });
}

std::string Js::SyncSubscription::subject() const noexcept
{
    return _impl->subject();
}

int64_t Js::SyncSubscription::id() const noexcept
{
    return _impl->id();
}

void Js::SyncSubscription::setAutoUnsubscribe(int messages) const
{
    _impl->setAutoUnsubscribe(messages);
}

void Js::SyncSubscription::setNoDeliveryDelay() const
{
    _impl->setNoDeliveryDelay();
}

void Js::SyncSubscription::unsubscribe() const
{
    _impl->unsubscribe();
}

Js::SubscriptionMismatch Js::SyncSubscription::mismatch() const
{
    return _impl->mismatch();
}

Js::Consumer Js::SyncSubscription::consumerInfo() const
{
    return _impl->consumerInfo();
}
