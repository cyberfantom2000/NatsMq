#pragma once

#include <memory>

#include "Entities.h"
#include "Export.h"

namespace NatsMq
{
    namespace Js
    {
        class StreamPrivate;

        class NATSMQ_EXPORT Stream
        {
        public:
            Stream(Js::StreamPrivate* impl);

            ~Stream();

            Stream(Stream&&);

            Stream& operator=(Stream&&);

            //! Update stream configuration
            void update(const Js::StreamConfig& options) const;

            //! Purge stream content
            void purge(const Js::Options::Stream::Purge& options) const;

            //! Remove stream
            void remove() const;

            //! Check stream exists
            bool exists() const;

            //! Get stream info
            StreamInfo info() const;

            // TODO consumer managment: add, list, delete, get

        private:
            std::unique_ptr<StreamPrivate> _impl;
        };
    }
}
