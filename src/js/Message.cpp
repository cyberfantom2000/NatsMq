#include "Message.h"

#include "MessagePrivate.h"

using namespace NatsMq::Js;

IncomingMessage::IncomingMessage(IncomingMessagePrivate* impl)
    : _impl(impl)
    , msg(_impl->message())
{
}

IncomingMessage::~IncomingMessage() = default;

IncomingMessage::IncomingMessage(IncomingMessage&&) = default;

IncomingMessage& IncomingMessage::operator=(IncomingMessage&&) = default;

IncomingMessage::operator std::string() const
{
    return msg;
}

void IncomingMessage::ack() const
{
    _impl->ack();
}

void IncomingMessage::nak(uint64_t delay) const
{
    _impl->nak(delay);
}

void IncomingMessage::inProgress() const
{
    _impl->inProgress();
}

void IncomingMessage::terminate() const
{
    _impl->terminate();
}

uint64_t IncomingMessage::sequnce() const noexcept
{
    return _impl->sequence();
}

int64_t IncomingMessage::timestamp() const noexcept
{
    return _impl->timestamp();
}
