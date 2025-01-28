#include "Exceptions.h"

#include <nats.h>

void NatsMq::exceptionIfError(NatsMq::Status status)
{
    exceptionIfError(static_cast<int>(status));
}

void NatsMq::exceptionIfError(int s)
{
    const auto status = static_cast<NatsMq::Status>(s);
    if (status == NatsMq::Status::Ok)
        return;
    else
        throw NatsMq::Exception(status);
}

void NatsMq::jsExceptionIfError(NatsMq::Status status, Js::Status error)
{
    jsExceptionIfError(static_cast<int>(status), static_cast<int>(error));
}

void NatsMq::jsExceptionIfError(int s, int jsError)
{
    const auto status = static_cast<NatsMq::Status>(s);
    if (status == NatsMq::Status::Ok)
        return;
    else
        throw NatsMq::JsException(s, jsError);
}

NatsMq::Exception::Exception(NatsMq::Status s)
    : std::runtime_error(natsStatus_GetText(static_cast<natsStatus>(s)))
    , status(s)
{
}

NatsMq::Exception::Exception(int s)
    : std::runtime_error(natsStatus_GetText(static_cast<natsStatus>(s)))
    , status(static_cast<NatsMq::Status>(s))
{
}

NatsMq::JsException::JsException(NatsMq::Status s, Js::Status e)
    : Exception(s)
    , jsError(e)
{
}

NatsMq::JsException::JsException(int s, int e)
    : Exception(s)
    , jsError(static_cast<Js::Status>(e))
{
}
