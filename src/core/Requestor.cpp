#include "Requestor.h"

#include "Entities.h"
#include "Exceptions.h"
#include "Message.h"
#include "private/utils.h"

using namespace NatsMq;

namespace
{
    using MsgPromise = std::promise<NatsMq::Message>;

    using NatsMsgPtr   = std::unique_ptr<natsMsg, decltype(&natsMsg_Destroy)>;
    using NatsInboxPtr = std::unique_ptr<natsInbox, decltype(&natsInbox_Destroy)>;
    using NatsSubPtr   = std::unique_ptr<natsSubscription, decltype(&natsSubscription_Destroy)>;

    std::string createUniqueInbox()
    {
        natsInbox* inbox{ nullptr };
        natsInbox_Create(&inbox);

        NatsInboxPtr ptr(inbox, &natsInbox_Destroy);

        return ptr.get();
    }

    void asyncRequestCallback(natsConnection* /*nc*/, natsSubscription* natsSub, natsMsg* cnatsMsg, void* closure)
    {
        std::unique_ptr<MsgPromise> promise(reinterpret_cast<MsgPromise*>(closure));

        NatsMq::NatsMsgPtr msg(cnatsMsg, &natsMsg_Destroy);
        NatsSubPtr         sub(natsSub, &natsSubscription_Destroy);

        try
        {
            if (!msg)
                throw NatsMq::Exception(NatsMq::Status::Timeout);
            if (natsMsg_IsNoResponders(cnatsMsg))
                throw NatsMq::Exception(NatsMq::Status::NoResponders);

            promise->set_value(fromCnatsMessage(msg.get()));
        }
        catch (...)
        {
            promise->set_exception(std::current_exception());
        }
    };
}

Requestor::Requestor(natsConnection* connection)
    : _connection(connection)
{
}

Message Requestor::request(Message msg, uint64_t timeoutMs) const
{
    const auto requestMsg = createCnatsMessage(msg);

    natsMsg*   replyMsg;
    const auto status = natsConnection_RequestMsg(&replyMsg, _connection, requestMsg.get(), timeoutMs);

    NatsMsgPtr replyPtr(replyMsg, &natsMsg_Destroy);

    exceptionIfError(status);

    return fromCnatsMessage(replyMsg);
}

std::future<Message> Requestor::arequest(Message msg, uint64_t timeoutMs) const
{
    auto promise = std::make_unique<MsgPromise>();

    const auto inbox = createUniqueInbox();

    natsSubscription* subscription{ nullptr };

    exceptionIfError(natsConnection_SubscribeTimeout(&subscription, _connection, inbox.c_str(), timeoutMs, &asyncRequestCallback, promise.get()));
    exceptionIfError(natsSubscription_AutoUnsubscribe(subscription, 1));
    exceptionIfError(natsConnection_PublishRequest(_connection, msg.subject.c_str(), inbox.c_str(), msg.data.data(), static_cast<int>(msg.data.size())));

    const auto rawPromise = promise.release();
    return rawPromise->get_future();
}
