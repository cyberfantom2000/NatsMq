#include "Stream.h"

#include "StreamPrivate.h"

using namespace NatsMq::Js;

Stream::Stream(StreamPrivate* impl)
    : _impl(impl)
{
}

Stream::~Stream() = default;

Stream::Stream(Stream&&) = default;

Stream& Stream::operator=(Stream&&) = default;

void Stream::update(const StreamConfig& options) const
{
    return _impl->update(options);
}

void Stream::purge(const Js::Options::Stream::Purge& options) const
{
    _impl->purge(options);
}

void Stream::remove() const
{
    _impl->remove();
}

bool Stream::exists() const
{
    return _impl->exists();
}

StreamInfo Stream::info() const
{
    return _impl->info();
}
