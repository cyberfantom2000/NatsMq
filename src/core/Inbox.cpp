#include "Inbox.h"

NatsMq::Inbox::Inbox()
    : _inbox(nullptr, &natsInbox_Destroy)
{
    natsInbox* inbox{ nullptr };
    natsInbox_Create(&inbox);
    _inbox.reset(inbox);
}

NatsMq::ByteArray NatsMq::Inbox::toByteArray() const
{
    return _inbox.get();
}

NatsMq::ByteArray NatsMq::Inbox::createByteArray()
{
    Inbox inbox;
    return inbox.toByteArray();
}
