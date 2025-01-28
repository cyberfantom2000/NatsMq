#include "SyncSubscriptionPrivate.h"

#include "Exceptions.h"
#include "Message.h"
#include "private/utils.h"

using namespace NatsMq;

SyncSubscriptionPrivate::SyncSubscriptionPrivate(natsConnection* connection, const std::string& subject)
    : SubscriptionBaseTemplate(natsConnection_SubscribeSync, connection, subject.c_str())
{
}

SyncSubscriptionPrivate::SyncSubscriptionPrivate(natsConnection* connection, const std::string& subject, const std::string& queue)
    : SubscriptionBaseTemplate(natsConnection_QueueSubscribeSync, connection, subject.c_str(), queue.c_str())
{
}

Message SyncSubscriptionPrivate::next(int64_t timeoutMs) const
{
    natsMsg* msg{ nullptr };
    exceptionIfError(natsSubscription_NextMsg(&msg, _sub.get(), timeoutMs));

    NatsMsgPtr ptr(msg, &natsMsg_Destroy);

    return fromCnatsMessage(msg);
}
