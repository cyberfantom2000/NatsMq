#include "Entities.h"

#include <opts.h>

#include <cstring>

NatsMq::Options::Options()
{
    randomize           = true;
    timeout             = NATS_OPTS_DEFAULT_TIMEOUT;
    secure              = false;
    verbose             = false;
    pedantic            = false;
    pingInterval        = NATS_OPTS_DEFAULT_PING_INTERVAL;
    maxPingsOut         = NATS_OPTS_DEFAULT_MAX_PING_OUT;
    ioBufferSize        = NATS_OPTS_DEFAULT_IO_BUF_SIZE;
    allowReconnect      = true;
    maxReconnect        = NATS_OPTS_DEFAULT_MAX_RECONNECT;
    reconnectWait       = NATS_OPTS_DEFAULT_RECONNECT_WAIT;
    reconnectBufferSize = NATS_OPTS_DEFAULT_RECONNECT_BUF_SIZE;
    maxPendingMessages  = NATS_OPTS_DEFAULT_MAX_PENDING_MSGS;
    echo                = true;
    sendAsap            = false;
}

NatsMq::ByteArray NatsMq::ByteArray::fromRawData(const char* data, int len)
{
    auto res = ByteArray(data, len);
    res._bytes.erase(res._bytes.end() - 1);
    return res;
}

NatsMq::ByteArray::ByteArray(const char* str)
{
    if (!str)
        return;

    const auto len = std::strlen(str) + 1;
    _bytes.resize(len);
    std::memcpy(_bytes.data(), str, len);
}

NatsMq::ByteArray::ByteArray(const char* data, int len)
{
    if (!data)
        return;

    _bytes.resize(len + 1);
    std::memcpy(_bytes.data(), data, len);
    _bytes[len] = '\0';
}

std::size_t NatsMq::ByteArray::size() const
{
    return _bytes.size();
}

std::string NatsMq::ByteArray::toString() const
{
    std::string s(_bytes.data());
    return s;
}

const char* NatsMq::ByteArray::constData() const
{
    return _bytes.data();
}
