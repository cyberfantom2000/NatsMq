#pragma once

#include <stdexcept>

#include "Entities.h"

namespace NatsMq
{
    void exceptionIfError(int status);

    void jsExceptionIfError(int status, int error);

    struct Exception : public std::runtime_error
    {
        Exception(Status);

        Exception(int);

        Status status;
    };

    struct JsException final : public Exception
    {
        JsException(Status, JsError);

        JsException(int, int);

        JsError jsError;
    };
}
