#pragma once

#include <memory>

#include "Entities.h"
#include "Export.h"

namespace NatsMq
{
    namespace Js
    {
        class ObjectWatcherPrivate;

        class NATSMQ_EXPORT ObjectWatcher
        {
        public:
            ObjectWatcher(ObjectWatcherPrivate* impl);

            ~ObjectWatcher();

            ObjectWatcher(ObjectWatcher&&);

            ObjectWatcher& operator=(ObjectWatcher&&);

            //! Stop watch
            void stop() const;

            //! Сalled whenever the file is updated
            void updated(ObjectWatchCb) const;

            //! Сalled when the file is deleted
            void removed(ObjectWatchCb) const;

        private:
            std::unique_ptr<ObjectWatcherPrivate> _impl;
        };

        //

        class ObjectStorePrivate;

        class NATSMQ_EXPORT ObjectStore
        {
        public:
            ObjectStore(ObjectStorePrivate* impl);

            ~ObjectStore();

            ObjectStore(ObjectStore&&);

            ObjectStore& operator=(ObjectStore&&);

            // TODO check object exists

            //! Put file to storage
            void put(const ObjectElement& object) const;

            //! Get file from storage
            ObjectElement get(const std::string& name, int64_t chunkTimeoutMs = 50) const;

            //! Get file meta from storage
            ObjectInfo info(const std::string& name) const;

            //! Remove file from storage
            void remove(const std::string& name) const;

            //! Get file watcher object
            ObjectWatcher* watch(const std::string& name) const;

            //! Check store exists
            bool storeExists() const;

            //! Delete store
            void deleteStore() const;

            Js::ObjectStoreConfig storeConfig() const;

        private:
            std::unique_ptr<ObjectStorePrivate> _impl;
        };
    }
}
