#pragma once

#include <nats.h>

#include "Entities.h"

namespace NatsMq
{
    class JsStreamImpl
    {
    public:
        static JsStreamImpl* getOrCreate(jsCtx* context, const JsStreamConfig& config);

        static JsStreamImpl* get(jsCtx* context, const JsStreamConfig& config);

        void update(const JsStreamConfig& config);

        void purge() const;

        void remove() const;

        bool exists() const;

    private:
        JsStreamImpl(jsCtx* _context, const JsStreamConfig& config);

        void create() const;

    private:
        jsCtx* _context;

        JsStreamConfig _config;
    };
}
