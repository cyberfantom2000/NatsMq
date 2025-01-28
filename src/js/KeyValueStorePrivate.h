#pragma once

#include <nats.h>

#include <memory>
#include <string>

#include "Entities.h"

namespace NatsMq
{
    namespace Js
    {
        using NatsKvPtr  = std::unique_ptr<kvStore, decltype(&kvStore_Destroy)>;
        using KvEntryPtr = std::unique_ptr<kvEntry, decltype(&kvEntry_Destroy)>;

        class KeyValueStorePrivate
        {
        public:
            static bool exists(jsCtx* ctx, const std::string& bucket);

            KeyValueStorePrivate(jsCtx* ctx, const KeyValue::Config& config);

            std::string bucket() const noexcept;

            KeyValue::Info info() const;

            std::vector<std::string> keys() const;

            KeyValue::Element get(const std::string& key) const;

            std::string getString(const std::string& key) const;

            void put(const std::string& key, const std::vector<uint8_t>& value, uint64_t revision) const;

            void put(const std::string& key, const std::string& value, uint64_t revision) const;

            void create(const std::string& key, const std::vector<uint8_t>& value, uint64_t revision) const;

            void create(const std::string& key, const std::string& value, uint64_t revision) const;

            void update(const std::string& key, const std::vector<uint8_t>& value, uint64_t newRevision, uint64_t expectedRevision);

            void update(const std::string& key, const std::string& value, uint64_t newRevision, uint64_t expectedRevision);

            // TODO purge options
            void purge(const std::string& key /*, KeyValue::PurgeOptions options*/) const;

            void remove(const std::string& key) const;

            void deleteStore() const;

            // TODO watcher.
            // Метод kvStore_Watch не позволяет регестрировать callback как подписка, а предлагает вызывать метод
            // kvWatcher_Next() и передавать ему таймаут для ожидания нового значения. Это поведение не очень подходит,
            // нужно реализовать что-то на подобии асинхронного отслеживания

        private:
            KvEntryPtr getKvEntry(const std::string& key) const;

        private:
            jsCtx*    _ctx;
            NatsKvPtr _kv;
        };

    }
}
