#pragma once

#include "MessageImpl.h"

namespace NatsMq
{
    struct JsIncomingMessageImpl final : IncomingMessageImpl
    {
        JsIncomingMessageImpl(natsMsg* msg);

        void ack() const;

        void nack(uint64_t delay = 0) const;

        void inProgress() const;

        void terminate() const;
    };
}
