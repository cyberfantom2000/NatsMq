#include "KeyValueStore.h"

#include "js/KeyValueStorePrivate.h"

using namespace NatsMq;

Js::KeyValueStore::KeyValueStore(KeyValueStorePrivate* impl)
    : _impl(impl)
{
}

Js::KeyValueStore::~KeyValueStore() = default;

Js::KeyValueStore::KeyValueStore(KeyValueStore&&) = default;

Js::KeyValueStore& Js::KeyValueStore::operator=(KeyValueStore&&) = default;

std::string Js::KeyValueStore::bucket() const
{
    return _impl->bucket();
}

Js::KeyValue::Info Js::KeyValueStore::info() const
{
    return _impl->info();
}

std::vector<std::string> Js::KeyValueStore::keys() const
{
    return _impl->keys();
}

Js::KeyValue::Element Js::KeyValueStore::get(const std::string& key) const
{
    return _impl->get(key);
}

std::string Js::KeyValueStore::getString(const std::string& key) const
{
    return _impl->getString(key);
}

void Js::KeyValueStore::put(const std::string& key, const std::vector<uint8_t>& value, uint64_t revison) const
{
    _impl->put(key, value, revison);
}

void Js::KeyValueStore::put(const std::string& key, const std::string& value, uint64_t revison) const
{
    _impl->put(key, value, revison);
}

void Js::KeyValueStore::create(const std::string& key, const std::vector<uint8_t>& value, uint64_t revison) const
{
    _impl->create(key, value, revison);
}

void Js::KeyValueStore::create(const std::string& key, const std::string& value, uint64_t revison) const
{
    _impl->create(key, value, revison);
}

void Js::KeyValueStore::update(const std::string& key, const std::vector<uint8_t>& value, uint64_t newRevision, uint64_t expectedRevision)
{
    _impl->update(key, value, newRevision, expectedRevision);
}

void Js::KeyValueStore::update(const std::string& key, const std::string& value, uint64_t newRevision, uint64_t expectedRevision)
{
    _impl->update(key, value, newRevision, expectedRevision);
}

void Js::KeyValueStore::purge(const std::string& key) const
{
    _impl->purge(key);
}

void Js::KeyValueStore::remove(const std::string& key) const
{
    _impl->purge(key);
}

void Js::KeyValueStore::deleteStore() const
{
    _impl->deleteStore();
}
