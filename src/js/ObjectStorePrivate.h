#pragma once

#include <nats.h>

#include "Entities.h"

namespace NatsMq
{
    namespace Js
    {
        class ObjectWatcherPrivate;

        class ObjectStorePrivate
        {
        public:
            static bool exists(jsCtx* ctx, const std::string& bucket);

            ObjectStorePrivate(jsCtx* ctx, const ObjectStoreConfig& config);

            ObjectInfo info(const std::string& name) const;

            // TODO maybe throw exception if object removed?
            ObjectElement get(const std::string& name, int64_t chunkTimeoutMs) const;

            // TODO async ?
            void put(const ObjectElement& object) const;

            void remove(const std::string& name) const;

            ObjectWatcherPrivate* watch(const std::string& name) const;

            bool storeExists() const;

            void deleteStore() const;

            ObjectStoreConfig storeConfig() const;

        private:
            jsCtx*      _ctx;
            std::string _bucket;
        };
    }
}
