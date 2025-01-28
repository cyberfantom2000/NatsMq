#include "PullSubscription.h"

#include "js/PullSubscriptionPrivate.h"

using namespace NatsMq;

Js::PullSubscription::PullSubscription(PullSubscriptionPrivate* impl)
    : _impl(impl)
{
}

Js::PullSubscription::~PullSubscription() = default;

Js::PullSubscription::PullSubscription(PullSubscription&&) = default;

Js::PullSubscription& Js::PullSubscription::operator=(PullSubscription&&) = default;

std::vector<Js::IncomingMessage> Js::PullSubscription::fetch(int batch, int64_t timeoutMs)
{
    return _impl->fetch(batch, timeoutMs);
}

SubscriptionStatistic Js::PullSubscription::statistics() const
{
    return _impl->statistics();
}

uint64_t Js::PullSubscription::messageCount() const
{
    return _impl->messageCount();
}

bool Js::PullSubscription::isValid() const noexcept
{
    return _impl->isValid();
}

void Js::PullSubscription::setPendingLimits(int message, int bytes) const
{
    return _impl->setPendingLimits({ message, bytes });
}

std::string Js::PullSubscription::subject() const noexcept
{
    return _impl->subject();
}

int64_t Js::PullSubscription::id() const noexcept
{
    return _impl->id();
}

void Js::PullSubscription::unsubscribe() const
{
    _impl->unsubscribe();
}
