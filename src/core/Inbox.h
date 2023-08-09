#pragma once

#include <nats.h>

#include <memory>

#include "Entities.h"
namespace NatsMq
{
    class Inbox
    {
    public:
        Inbox();

        ByteArray toByteArray() const;

        static ByteArray createByteArray();

    private:
        using NatsInboxPtr = std::unique_ptr<natsInbox, decltype(&natsInbox_Destroy)>;

        NatsInboxPtr _inbox;
    };
}
