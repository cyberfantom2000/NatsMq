#include "Utils.h"

#include "Exceptions.h"
#include "Message.h"

NatsMq::NatsMsgPtr NatsMq::createNatsMessage(const NatsMq::Message& msg)
{
    natsMsg* cnatsMsg;

    const auto realReply = msg.reply.size() ? msg.reply.c_str() : nullptr;
    const auto dataSize  = static_cast<int>(msg.data.size());

    exceptionIfError(natsMsg_Create(&cnatsMsg, msg.subject.c_str(), realReply, msg.data.constData(), dataSize));

    NatsMsgPtr natsMsg(cnatsMsg, &natsMsg_Destroy);

    for (auto it = msg.headers.cbegin(), ite = msg.headers.cend(); it != ite; ++it)
        exceptionIfError(natsMsgHeader_Add(cnatsMsg, it->first.c_str(), it->second.c_str()));

    return natsMsg;
}

bool NatsMq::makePing(natsConnection* connection, int timeout)
{
    const auto status = static_cast<Status>(natsConnection_FlushTimeout(connection, timeout));
    return status == Status::Ok;
}

void NatsMq::configurePoolSize(int poolSize)
{
    if (poolSize > 1)
        nats_SetMessageDeliveryPoolSize(poolSize);
}

std::string NatsMq::emptyStringIfNull(const char* s)
{
    return s ? s : "";
}

NatsMq::Message NatsMq::fromCnatsMessage(natsMsg* msg)
{
    NatsMq::Message out;
    out.subject = natsMsg_GetSubject(msg);
    out.reply   = NatsMq::emptyStringIfNull(natsMsg_GetReply(msg));
    out.data    = NatsMq::ByteArray(natsMsg_GetData(msg), natsMsg_GetDataLength(msg));
}
