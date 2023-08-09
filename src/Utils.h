#pragma once

#include <nats.h>

#include <memory>
#include <string>

namespace NatsMq
{
    using NatsMsgPtr = std::unique_ptr<natsMsg, decltype(&natsMsg_Destroy)>;

    class Message;

    NatsMq::NatsMsgPtr createNatsMessage(const Message& msg);

    bool makePing(natsConnection*, int timeout);

    void configurePoolSize(int poolSize);

    std::string emptyStringIfNull(const char* s);

    NatsMq::Message fromCnatsMessage(natsMsg* msg);
}
