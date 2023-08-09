#include <nats.h>

#include <memory>

#include "Message.h"

namespace NatsMq
{
    using NatsMsgPtr = std::unique_ptr<natsMsg, decltype(&natsMsg_Destroy)>;

    struct IncomingMessageImpl
    {
        IncomingMessageImpl(natsMsg* msg);

        Message::Headers parseHeaders();

        std::string subject;
        std::string reply;
        ByteArray   data;

    protected:
        NatsMsgPtr _msg;
    };
}
