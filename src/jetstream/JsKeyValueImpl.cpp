#include "JsKeyValueImpl.h"

#include "Exceptions.h"
#include "JsKeyValue.h"

namespace
{
    using KvEntryPtr = std::unique_ptr<kvEntry, decltype(&kvEntry_Destroy)>;

    kvConfig createNatsConfig(const NatsMq::KeyValueConfig& config)
    {
        kvConfig natsConfig;
        kvConfig_Init(&natsConfig);

        natsConfig.Bucket       = config.name.c_str();
        natsConfig.History      = config.history;
        natsConfig.MaxBytes     = config.maxBytes.value_or(natsConfig.MaxBytes);
        natsConfig.MaxValueSize = config.maxValueSize.value_or(natsConfig.MaxValueSize);

        return natsConfig;
    }
}

NatsMq::KeyValueStore::KeyValueStore(KeyValueStoreImpl* impl)
    : _impl(impl)
{
}

void NatsMq::KeyValueStore::deleteStore() const
{
    _impl->deleteStore();
}

NatsMq::KeyValueStore::~KeyValueStore() = default;

std::string NatsMq::KeyValueStore::storeName() const
{
    return _impl->storeName();
}

std::vector<std::string> NatsMq::KeyValueStore::keys() const
{
    return _impl->keys();
}

NatsMq::ByteArray NatsMq::KeyValueStore::getElement(const std::string& key) const
{
    return _impl->getElement(key);
}

void NatsMq::KeyValueStore::putElement(const std::string& key, const ByteArray& value) const
{
    return _impl->putElement(key, value);
}

void NatsMq::KeyValueStore::createElement(const std::string& key, const ByteArray& value) const
{
    return _impl->createElement(key, value);
}

void NatsMq::KeyValueStore::purgeElement(const std::string& key) const
{
    return _impl->purgeElement(key);
}

void NatsMq::KeyValueStore::removeElement(const std::string& key) const
{
    return _impl->removeElement(key);
}

NatsMq::KeyValueStoreImpl* NatsMq::KeyValueStoreImpl::getOrCreate(jsCtx* context, const KeyValueConfig& config)
{
    try
    {
        return KeyValueStoreImpl::get(context, config);
    }
    catch (const Exception& exc)
    {
        if (exc.status != Status::NotFound)
            throw;
        kvStore* natsKv     = nullptr;
        auto     natsConfig = createNatsConfig(config);
        exceptionIfError(js_CreateKeyValue(&natsKv, context, &natsConfig));
        return new KeyValueStoreImpl(natsKv, context);
    }
}

NatsMq::KeyValueStoreImpl* NatsMq::KeyValueStoreImpl::get(jsCtx* context, const KeyValueConfig& config)
{
    kvStore*   kv     = nullptr;
    const auto status = js_KeyValue(&kv, context, config.name.c_str());

    exceptionIfError(status);

    KeyValueStorePtr kvPtr(kv, &kvStore_Destroy);
    return new KeyValueStoreImpl(kvPtr.release(), context);
}

void NatsMq::KeyValueStoreImpl::deleteStore() const
{
    const auto name = storeName();
    exceptionIfError(js_DeleteKeyValue(_context, name.c_str()));
}

NatsMq::KeyValueStoreImpl::KeyValueStoreImpl(kvStore* kv, jsCtx* context)
    : _context(context)
    , _kv(kv, &kvStore_Destroy)
{
}

std::string NatsMq::KeyValueStoreImpl::storeName() const
{
    return kvStore_Bucket(_kv.get());
}

std::vector<std::string> NatsMq::KeyValueStoreImpl::keys() const
{
    kvKeysList list;
    exceptionIfError(kvStore_Keys(&list, _kv.get(), nullptr));

    std::vector<std::string> out;
    for (auto i = 0; i < list.Count; ++i)
        out.emplace_back(list.Keys[i]);

    kvKeysList_Destroy(&list);
    return out;
}

NatsMq::ByteArray NatsMq::KeyValueStoreImpl::getElement(const std::string& key) const
{
    kvEntry* natsEntry = nullptr;
    exceptionIfError(kvStore_Get(&natsEntry, _kv.get(), key.c_str()));
    KvEntryPtr entry(natsEntry, &kvEntry_Destroy);

    const auto data = reinterpret_cast<const char*>(kvEntry_Value(natsEntry));
    return ByteArray::fromRawData(data, kvEntry_ValueLen(natsEntry));
}

void NatsMq::KeyValueStoreImpl::putElement(const std::string& key, const ByteArray& value) const
{
    exceptionIfError(kvStore_Put(nullptr, _kv.get(), key.c_str(), value.constData(), static_cast<int>(value.size())));
}

void NatsMq::KeyValueStoreImpl::createElement(const std::string& key, const ByteArray& value) const
{
    exceptionIfError(kvStore_Create(nullptr, _kv.get(), key.c_str(), value.constData(), static_cast<int>(value.size())));
}

void NatsMq::KeyValueStoreImpl::purgeElement(const std::string& key) const
{
    exceptionIfError(kvStore_Purge(_kv.get(), key.c_str(), nullptr));
}

void NatsMq::KeyValueStoreImpl::removeElement(const std::string& key) const
{
    exceptionIfError(kvStore_Delete(_kv.get(), key.c_str()));
}
