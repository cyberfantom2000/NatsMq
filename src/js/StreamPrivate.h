#pragma once

#include <nats.h>

#include "Entities.h"

namespace NatsMq
{
    namespace Js
    {
        class StreamPrivate
        {
        public:
            StreamPrivate(jsCtx* context, const std::string& name) noexcept;

            void create(const Js::StreamConfig& config) const;

            bool exists() const;

            Js::StreamInfo info() const;

            void purge(const Js::Options::Stream::Purge& options) const;

            void remove() const;

            void update(const Js::StreamConfig& config) const;

            static bool exists(jsCtx* context, const std::string& name);

            static Js::StreamInfo info(jsCtx* context, const std::string& name);

            static std::vector<Js::StreamInfo> infos(jsCtx* context);

            static std::vector<std::string> names(jsCtx* context);

        private:
            std::string _name;
            jsCtx*      _context;
        };
    }
}
