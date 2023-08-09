#pragma once

#include <nats.h>

#include "Entities.h"

namespace NatsMq
{
    class KeyValueStoreImpl
    {
    public:
        using KeyValueStorePtr = std::unique_ptr<kvStore, decltype(&kvStore_Destroy)>;

        static KeyValueStoreImpl* getOrCreate(jsCtx* context, const KeyValueConfig& config);

        static KeyValueStoreImpl* get(jsCtx* context, const KeyValueConfig& config);

        void deleteStore() const;

        std::string storeName() const;

        std::vector<std::string> keys() const;

        ByteArray getElement(const std::string& key) const;

        void putElement(const std::string& key, const ByteArray& value) const;

        void createElement(const std::string& key, const ByteArray& value) const;

        void purgeElement(const std::string& key) const;

        void removeElement(const std::string& key) const;

    private:
        KeyValueStoreImpl(kvStore* kv, jsCtx* context);

    private:
        jsCtx*           _context;
        KeyValueStorePtr _kv;
    };
}
