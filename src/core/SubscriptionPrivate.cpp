#include "SubscriptionPrivate.h"

#include "Exceptions.h"
#include "Message.h"
#include "private/utils.h"

using namespace NatsMq;

SubscriptionPrivate::SubscriptionPrivate(natsConnection* connection, const std::string& subject)
    : SubscriptionBaseTemplate(&natsConnection_Subscribe, connection, subject.c_str(), &subscriptionCallback, this)
{
}

SubscriptionPrivate::SubscriptionPrivate(natsConnection* connection, const std::string& subject, int64_t timeoutMs)
    : SubscriptionBaseTemplate(&natsConnection_SubscribeTimeout, connection, subject.c_str(), timeoutMs, &subscriptionCallback, this)
{
}

SubscriptionPrivate::SubscriptionPrivate(natsConnection* connection, const std::string& subject, const std::string& queue)
    : SubscriptionPrivate(connection, subject.c_str(), queue.c_str(), 0)
{
}

SubscriptionPrivate::SubscriptionPrivate(natsConnection* connection, const std::string& subject, const std::string& queue, int64_t timeoutMs)
    : SubscriptionBaseTemplate(&natsConnection_QueueSubscribeTimeout, connection, subject.c_str(), queue.c_str(), timeoutMs, &subscriptionCallback, this)
{
}

void SubscriptionPrivate::registerListener(SubscriptionCb cb)
{
    _cb = std::move(cb);
}

void SubscriptionPrivate::drain(int64_t timeout) const
{
    if (timeout < 0)
        exceptionIfError(natsSubscription_Drain(_sub.get()));
    else
        exceptionIfError(natsSubscription_DrainTimeout(_sub.get(), timeout));
}

void SubscriptionPrivate::waitDrain(int64_t timeoutMs) const
{

    exceptionIfError(natsSubscription_WaitForDrainCompletion(_sub.get(), timeoutMs));
}

Status SubscriptionPrivate::drainStatus() const
{
    return static_cast<Status>(natsSubscription_DrainCompletionStatus(_sub.get()));
}

void SubscriptionPrivate::subscriptionCallback(natsConnection*, natsSubscription*, natsMsg* msg, void* closure)
{
    const auto sub = reinterpret_cast<SubscriptionPrivate*>(closure);
    NatsMsgPtr ptr(msg, &natsMsg_Destroy);
    sub->dataReady(fromCnatsMessage(msg));
}

void SubscriptionPrivate::dataReady(Message msg)
{
    _cb(std::move(msg));
}
