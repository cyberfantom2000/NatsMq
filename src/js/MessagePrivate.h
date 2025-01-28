#pragma once

#include "Entities.h"
#include "private/defines.h"

namespace NatsMq
{
    namespace Js
    {
        struct IncomingMessagePrivate
        {
            IncomingMessagePrivate(natsMsg* msg);

            void ack() const;

            void nak(uint64_t delay) const;

            void inProgress() const;

            void terminate() const;

            uint64_t sequence() const noexcept;

            int64_t timestamp() const noexcept;

            MessageMeta meta() const;

            Message message() const;

        private:
            NatsMsgPtr _msg;
        };
    }
}
