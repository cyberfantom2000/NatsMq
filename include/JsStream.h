#pragma once

#include "Entities.h"
#include "natsmq_export.h"

namespace NatsMq
{
    class JsStreamImpl;

    class NATSMQ_EXPORT JsStream
    {
    public:
        JsStream() = default;

        JsStream(JsStreamImpl*);

        JsStream(JsStream&&);

        JsStream& operator=(JsStream&&);

        ~JsStream();

        void update(const JsStreamConfig& config);

        void purge() const;

        //! Delete stream on server
        void remove() const;

        bool exists() const;

    private:
        std::unique_ptr<JsStreamImpl> _impl;
    };
}
