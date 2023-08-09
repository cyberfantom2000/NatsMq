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

    private:
        JsStreamImpl(jsCtx* _context, const JsStreamConfig& config);

        bool exists() const;

        void create() const;

    private:
        jsCtx* _context;

        JsStreamConfig _config;
    };
}
