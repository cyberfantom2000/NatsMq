#include "MessageManagerPrivate.h"

#include "Exceptions.h"
#include "Message.h"
#include "private/utils.h"

using namespace NatsMq;

Js::MessageManagerPrivate::MessageManagerPrivate(jsCtx* ctx)
    : _ctx(ctx)
{
}

Message Js::MessageManagerPrivate::getMessage(const std::string& stream, uint64_t sequence) const
{
    natsMsg* msg{ nullptr };

    jsErrCode  jerr;
    const auto status = js_GetMsg(&msg, _ctx, stream.c_str(), sequence, nullptr, &jerr);

    jsExceptionIfError(status, jerr);

    NatsMsgPtr ptr(msg, &natsMsg_Destroy);

    return fromCnatsMessage(msg);
}

Message Js::MessageManagerPrivate::getLastMessage(const std::string& stream, const std::string& subject) const
{
    natsMsg* msg{ nullptr };

    jsErrCode  jerr;
    const auto status = js_GetLastMsg(&msg, _ctx, stream.c_str(), subject.c_str(), nullptr, &jerr);

    jsExceptionIfError(status, jerr);

    NatsMsgPtr ptr(msg, &natsMsg_Destroy);

    return fromCnatsMessage(msg);
}

void Js::MessageManagerPrivate::deleteMessage(const std::string& stream, uint64_t sequence) const
{
    jsErrCode  jerr;
    const auto status = js_DeleteMsg(_ctx, stream.c_str(), sequence, nullptr, &jerr);
    jsExceptionIfError(status, jerr);
}

void Js::MessageManagerPrivate::eraseMessage(const std::string& stream, uint64_t sequence) const
{
    jsErrCode  jerr;
    const auto status = js_EraseMsg(_ctx, stream.c_str(), sequence, nullptr, &jerr);
    jsExceptionIfError(status, jerr);
}
