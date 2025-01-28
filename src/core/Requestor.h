#pragma once

#include <nats.h>

#include <future>

namespace NatsMq
{
    class Message;

    class Requestor
    {
    public:
        Requestor(natsConnection* connection);

        Message request(Message msg, uint64_t timeoutMs) const;

        std::future<Message> arequest(Message msg, uint64_t timeoutMs) const;

    private:
        natsConnection* _connection;
    };
}
