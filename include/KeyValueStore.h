#pragma once

#include <memory>
#include <vector>

#include "Entities.h"
#include "Export.h"

namespace NatsMq
{
    namespace Js
    {
        class KeyValueStorePrivate;

        class NATSMQ_EXPORT KeyValueStore
        {
        public:
            KeyValueStore(KeyValueStorePrivate* impl);

            ~KeyValueStore();

            KeyValueStore(KeyValueStore&&);

            KeyValueStore& operator=(KeyValueStore&&);

            //! Get bucket name
            std::string bucket() const;

            //! Get key-value store info
            Js::KeyValue::Info info() const;

            //! Get all store keys
            std::vector<std::string> keys() const;

            //! Return key-value element. The NULL terminating character is not included in the number of bytes stored in the KeyValue store
            KeyValue::Element get(const std::string& key) const;

            //! Same the get(string), but return string
            std::string getString(const std::string& key) const;

            //! Places the new value for the key into the store. if the revision=0, it is ignored
            void put(const std::string& key, const std::vector<uint8_t>& value, uint64_t revison = 0) const;

            //! Same the put(string, vector) for easy string place. if the revision=0, it is ignored
            void put(const std::string& key, const std::string& value, uint64_t revison = 0) const;

            //! Places the value for the key into the store if and only if the key does not exist. if the revision=0, it is ignored
            void create(const std::string& key, const std::vector<uint8_t>& value, uint64_t revison = 0) const;

            //! Same the create(string, vector) for easy string place. if the revision=0, it is ignored
            void create(const std::string& key, const std::string& value, uint64_t revison = 0) const;

            //! Updates the value for the key into the store if and only if the latest revision matches.
            void update(const std::string& key, const std::vector<uint8_t>& value, uint64_t newRevision, uint64_t expectedRevision);

            //! Same update(string, vector, uint64_t, uint64_t) for easy string place.
            void update(const std::string& key, const std::string& value, uint64_t newRevision, uint64_t expectedRevision);

            //! Deletes a key by placing a purge marker and removing all revisions.
            void purge(const std::string& key) const;

            //! Deletes a key by placing a delete marker and leaving all revisions.
            void remove(const std::string& key) const;

            void deleteStore() const;

            // TODO history

        private:
            std::unique_ptr<KeyValueStorePrivate> _impl;
        };

    }
}
