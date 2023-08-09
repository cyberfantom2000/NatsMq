#include "Request.h"

#include "Exceptions.h"
#include "Inbox.h"
#include "MessageImpl.h"
#include "Utils.h"

namespace
{
    using IncomingMsgPromise = std::promise<NatsMq::IncomingMessage>;
    using NatsSubPtr         = std::unique_ptr<natsSubscription, decltype(&natsSubscription_Destroy)>;

    void asyncRequestCallback(natsConnection* /*nc*/, natsSubscription* natsSub, natsMsg* cnatsMsg, void* closure)
    {
        std::unique_ptr<IncomingMsgPromise> promise(reinterpret_cast<IncomingMsgPromise*>(closure));

        NatsMq::NatsMsgPtr msg(cnatsMsg, &natsMsg_Destroy);
        NatsSubPtr         sub(natsSub, &natsSubscription_Destroy);

        try
        {
            if (msg)
            {
                if (natsMsg_IsNoResponders(cnatsMsg))
                    throw NatsMq::Exception(NatsMq::Status::NoResponders);
                else
                {
                    NatsMq::IncomingMessageImpl privateMsg(msg.release());
                    promise->set_value(NatsMq::IncomingMessage(&privateMsg));
                }
            }
            else
                throw NatsMq::Exception(NatsMq::Status::Timeout);
        }
        catch (...)
        {
            promise->set_exception(std::current_exception());
        }
    };
}

NatsMq::Request::Request(natsConnection* connection)
    : _connection(connection)
{
}

NatsMq::IncomingMessage NatsMq::Request::make(const Message& msg, uint64_t timeoutMs)
{
    const NatsMsgPtr reqestMsg = createNatsMessage(msg);

    natsMsg* replyMsg;
    exceptionIfError(natsConnection_RequestMsg(&replyMsg, _connection, reqestMsg.get(), timeoutMs));
    IncomingMessageImpl privateMsg(replyMsg);

    return IncomingMessage(&privateMsg);
}

std::future<NatsMq::IncomingMessage> NatsMq::Request::asyncMake(const Message& msg, uint64_t timeoutMs)
{
    auto       promise = std::make_unique<IncomingMsgPromise>();
    const auto inbox   = Inbox::createByteArray();

    natsSubscription* subscription{ nullptr };
    exceptionIfError(natsConnection_SubscribeTimeout(&subscription, _connection, inbox.constData(), timeoutMs, &asyncRequestCallback, promise.get()));
    exceptionIfError(natsSubscription_AutoUnsubscribe(subscription, 1));
    exceptionIfError(natsConnection_PublishRequest(_connection, msg.subject.c_str(), inbox.constData(), msg.data.constData(), static_cast<int>(msg.data.size())));

    const auto rawPromise = promise.release();
    return rawPromise->get_future();
}
