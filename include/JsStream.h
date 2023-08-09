#pragma once

#include "Entities.h"

namespace NatsMq
{
    class JsStreamImpl;

    class JsStream
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

    private:
        std::unique_ptr<JsStreamImpl> _impl;
    };
}
