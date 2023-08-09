#pragma once

#include <nats.h>

namespace NatsMq
{
    class Message;

    class Publisher
    {
    public:
        Publisher(natsConnection* connection);

        void puslish(const Message& msg);

    private:
        natsConnection* _connection;
    };

}
