#include "MessageImpl.h"

#include "Utils.h"

NatsMq::IncomingMessage::IncomingMessage(IncomingMessageImpl* msg)
    : Message(msg->subject, msg->data)
{
    reply   = msg->reply;
    headers = msg->parseHeaders();
}

NatsMq::IncomingMessageImpl::IncomingMessageImpl(natsMsg* msg)
    : _msg(msg, &natsMsg_Destroy)
{
    subject = emptyStringIfNull(natsMsg_GetSubject(msg));
    data    = ByteArray(natsMsg_GetData(msg), natsMsg_GetDataLength(msg));
    reply   = emptyStringIfNull(natsMsg_GetReply(msg));
}

NatsMq::Message::Headers NatsMq::IncomingMessageImpl::parseHeaders()
{
    const auto cnatsMsg = _msg.get();

    const char** keys     = nullptr;
    int          keyCount = 0;

    natsStatus s = natsMsgHeader_Keys(cnatsMsg, &keys, &keyCount);
    if (s != NATS_OK || keyCount == 0)
        return {};

    Message::Headers headers;
    // handle message headers
    for (int i = 0; i < keyCount; i++)
    {
        const char** values{ nullptr };
        int          valueCount{ 0 };
        s = natsMsgHeader_Values(cnatsMsg, keys[i], &values, &valueCount);
        if (s == NATS_OK)
        {
            std::string key(keys[i]);

            for (int j = 0; j < valueCount; j++)
            {
                std::string value(values[j]);
                headers.insert(std::make_pair(key, value));
            }
            free(values);
        }
    }

    free(keys);

    return headers;
}
