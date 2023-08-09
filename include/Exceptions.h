#pragma once

#include <stdexcept>

#include "Entities.h"

namespace NatsMq
{
    void exceptionIfError(int status);

    void jsExceptionIfError(int status, int error = static_cast<int>(JsError::NoJsError));

    struct Exception : public std::runtime_error
    {
        Exception(Status);

        Exception(int);

        Status status;
    };

    struct JsException final : public Exception
    {
        JsException(Status s, JsError e);

        JsException(int s, int e);

        JsError jsError;
    };
}
