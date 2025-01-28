#pragma once

#include <nats.h>

#include <string>

namespace NatsMq
{
    class Message;

    class Publisher
    {
    public:
        Publisher(natsConnection* connection);

        void publish(Message msg) const;

        void publish(std::string subject, std::string data) const;

    private:
        natsConnection* _connection;
    };
}
