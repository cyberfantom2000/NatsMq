#pragma once

#include <stdexcept>

#include "Entities.h"
#include "natsmq_export.h"

namespace NatsMq
{
    void NATSMQ_EXPORT exceptionIfError(int status);

    void NATSMQ_EXPORT jsExceptionIfError(int status, int error = static_cast<int>(JsError::NoJsError));

    struct NATSMQ_EXPORT Exception : public std::runtime_error
    {
        Exception(Status);

        Exception(int);

        Status status;
    };

    struct NATSMQ_EXPORT JsException final : public Exception
    {
        JsException(Status s, JsError e);

        JsException(int s, int e);

        JsError jsError;
    };
}
