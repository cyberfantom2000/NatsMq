#include "Exceptions.h"

using namespace NatsMq;

#include <nats.h>

void NatsMq::exceptionIfError(int s)
{
    const auto status = static_cast<Status>(s);
    if (status == Status::Ok)
        return;
    else
        throw NatsMq::Exception(status);
}

void NatsMq::jsExceptionIfError(int s, int jsError)
{
    const auto status = static_cast<Status>(s);
    if (status == Status::Ok)
        return;
    else
        throw NatsMq::JsException(s, jsError);
}

NatsMq::Exception::Exception(Status s)
    : std::runtime_error(natsStatus_GetText(static_cast<natsStatus>(s)))
    , status(s)
{
}

NatsMq::Exception::Exception(int s)
    : std::runtime_error(natsStatus_GetText(static_cast<natsStatus>(s)))
    , status(static_cast<Status>(s))
{
}

JsException::JsException(Status s, JsError e)
    : Exception(s)
    , jsError(e)
{
}

JsException::JsException(int s, int e)
    : Exception(s)
    , jsError(static_cast<JsError>(e))
{
}
