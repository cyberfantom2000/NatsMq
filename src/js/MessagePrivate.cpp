#include "MessagePrivate.h"

#include "Exceptions.h"
#include "Message.h"
#include "private/utils.h"

using namespace NatsMq::Js;

namespace
{
    using NatsMetaPtr = std::unique_ptr<jsMsgMetaData, decltype(&jsMsgMetaData_Destroy)>;

    MessageMeta fromCnatsMeta(jsMsgMetaData* meta)
    {
        MessageMeta result;

        result.delivered = meta->NumDelivered;
        result.pending   = meta->NumPending;
        result.timestamp = meta->Timestamp;
        result.stream    = meta->Stream;
        result.consumer  = meta->Consumer;
        result.domain    = meta->Domain;

        result.sequence.consumer = meta->Sequence.Consumer;
        result.sequence.stream   = meta->Sequence.Stream;

        return result;
    }
}

IncomingMessagePrivate::IncomingMessagePrivate(natsMsg* msg)
    : _msg(msg, &natsMsg_Destroy)
{
}

void IncomingMessagePrivate::ack() const
{
    jsErrCode  jsErr;
    const auto status = natsMsg_AckSync(_msg.get(), nullptr, &jsErr);
    jsExceptionIfError(status, jsErr);
}

void IncomingMessagePrivate::nak(uint64_t delay) const
{
    const auto status = delay ? natsMsg_NakWithDelay(_msg.get(), delay, nullptr) : natsMsg_Nak(_msg.get(), nullptr);
    jsExceptionIfError(status);
}

void IncomingMessagePrivate::inProgress() const
{
    jsExceptionIfError(natsMsg_InProgress(_msg.get(), nullptr));
}

void IncomingMessagePrivate::terminate() const
{
    jsExceptionIfError(natsMsg_Term(_msg.get(), nullptr));
}

uint64_t IncomingMessagePrivate::sequence() const noexcept
{
    return natsMsg_GetSequence(_msg.get());
}

int64_t IncomingMessagePrivate::timestamp() const noexcept
{
    return natsMsg_GetTime(_msg.get());
}

MessageMeta IncomingMessagePrivate::meta() const
{
    jsMsgMetaData* meta{ nullptr };
    const auto     status = natsMsg_GetMetaData(&meta, _msg.get());

    jsExceptionIfError(status);

    NatsMetaPtr metaPtr(meta, &jsMsgMetaData_Destroy);

    return fromCnatsMeta(meta);
}

NatsMq::Message IncomingMessagePrivate::message() const
{
    return fromCnatsMessage(_msg.get());
}
