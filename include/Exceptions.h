#pragma once

#include <stdexcept>

#include "Entities.h"
#include "Export.h"

namespace NatsMq
{
    void NATSMQ_EXPORT exceptionIfError(NatsMq::Status status);

    void NATSMQ_EXPORT exceptionIfError(int status);

    void NATSMQ_EXPORT jsExceptionIfError(NatsMq::Status status, NatsMq::Js::Status error = Js::Status::NoJsError);

    void NATSMQ_EXPORT jsExceptionIfError(int status, int error = static_cast<int>(Js::Status::NoJsError));

    struct NATSMQ_EXPORT Exception : public std::runtime_error
    {
        Exception(NatsMq::Status);

        Exception(int);

        NatsMq::Status status;
    };

    struct NATSMQ_EXPORT JsException final : public Exception
    {
        JsException(NatsMq::Status s, Js::Status e);

        JsException(int s, int e);

        Js::Status jsError;
    };
}
