#include "JsMessage.h"

#include <nats.h>

#include "Exceptions.h"

NatsMq::JsIncomingMessageImpl::JsIncomingMessageImpl(natsMsg* msg)
    : IncomingMessageImpl(msg)
{
}

void NatsMq::JsIncomingMessageImpl::ack() const
{
    jsErrCode  jsErr;
    const auto status = natsMsg_AckSync(_msg.get(), nullptr, &jsErr);
    jsExceptionIfError(status, jsErr);
}

void NatsMq::JsIncomingMessageImpl::nack(uint64_t delay) const
{
    const auto status = delay ? natsMsg_NakWithDelay(_msg.get(), delay, nullptr) : natsMsg_Nak(_msg.get(), nullptr);
    jsExceptionIfError(status);
}

void NatsMq::JsIncomingMessageImpl::inProgress() const
{
    jsExceptionIfError(natsMsg_InProgress(_msg.get(), nullptr));
}

void NatsMq::JsIncomingMessageImpl::terminate() const
{
    jsExceptionIfError(natsMsg_Term(_msg.get(), nullptr));
}

NatsMq::JsIncomingMessage::JsIncomingMessage(JsIncomingMessageImpl* msg)
    : IncomingMessage(msg)
    , _impl(msg)
{
}

NatsMq::JsIncomingMessage::~JsIncomingMessage() = default;

NatsMq::JsIncomingMessage::JsIncomingMessage(JsIncomingMessage&&) noexcept = default;

NatsMq::JsIncomingMessage& NatsMq::JsIncomingMessage::operator=(JsIncomingMessage&&) noexcept = default;

void NatsMq::JsIncomingMessage::ack() const
{
    _impl->ack();
}

void NatsMq::JsIncomingMessage::nack(uint64_t delay) const
{
    _impl->nack(delay);
}

void NatsMq::JsIncomingMessage::inProgress() const
{
    _impl->inProgress();
}

void NatsMq::JsIncomingMessage::terminate() const
{
    _impl->terminate();
}
