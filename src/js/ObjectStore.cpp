#include "ObjectStore.h"

#include "js/ObjectStorePrivate.h"
#include "js/ObjectWatcherPrivate.h"

using namespace NatsMq;

Js::ObjectWatcher::ObjectWatcher(ObjectWatcherPrivate* impl)
    : _impl(impl)
{
}

Js::ObjectWatcher::~ObjectWatcher() = default;

Js::ObjectWatcher::ObjectWatcher(ObjectWatcher&&) = default;

Js::ObjectWatcher& Js::ObjectWatcher::operator=(ObjectWatcher&&) = default;

void Js::ObjectWatcher::stop() const
{
    _impl->stop();
}

void Js::ObjectWatcher::updated(ObjectWatchCb cb) const
{
    _impl->updated(std::move(cb));
}

void Js::ObjectWatcher::removed(ObjectWatchCb cb) const
{
    _impl->removed(std::move(cb));
}

//
//

Js::ObjectStore::ObjectStore(ObjectStorePrivate* impl)
    : _impl(impl)
{
}

Js::ObjectStore::~ObjectStore() = default;

Js::ObjectStore::ObjectStore(ObjectStore&&) = default;

Js::ObjectStore& Js::ObjectStore::operator=(ObjectStore&&) = default;

void Js::ObjectStore::put(const ObjectElement& object) const
{
    _impl->put(object);
}

Js::ObjectElement Js::ObjectStore::get(const std::string& name, int64_t chunkTimeoutMs) const
{
    return _impl->get(name, chunkTimeoutMs);
}

Js::ObjectInfo Js::ObjectStore::info(const std::string& name) const
{
    return _impl->info(name);
}

void Js::ObjectStore::remove(const std::string& name) const
{
    _impl->remove(name);
}

Js::ObjectWatcher* Js::ObjectStore::watch(const std::string& name) const
{
    return new ObjectWatcher(_impl->watch(name));
}

bool Js::ObjectStore::storeExists() const
{
    return _impl->storeExists();
}

void Js::ObjectStore::deleteStore() const
{
    _impl->deleteStore();
}

Js::ObjectStoreConfig Js::ObjectStore::storeConfig() const
{
    return _impl->storeConfig();
}
