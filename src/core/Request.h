#pragma once

#include <nats.h>

#include <future>

#include "Message.h"

namespace NatsMq
{
    struct Request
    {
        Request(natsConnection* connection);

        IncomingMessage make(const Message& msg, uint64_t timeoutMs);

        std::future<IncomingMessage> asyncMake(const Message& msg, uint64_t timeoutMs);

    private:
        natsConnection* _connection;
    };
}
