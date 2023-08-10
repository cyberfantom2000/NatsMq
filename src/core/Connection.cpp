#include "Connection.h"

#include <opts.h>

#include "Entities.h"
#include "Exceptions.h"

using namespace NatsMq;

namespace
{
    std::vector<const char*> createArrayPointersToElements(const Connection::Urls& elements)
    {
        std::vector<const char*> pointers;
        for (auto&& url : elements)
            pointers.push_back(url.c_str());

        return pointers;
    }
}

NatsMq::Connection::Connection()
    : _connection(nullptr)
{
    auto natsOptions = _options.rawOptions();
    setErrorHandler(natsOptions);
    setConnectionHandlers(natsOptions);
}

NatsMq::Connection::~Connection()
{
    disconnect();
}

ConnectionStatus Connection::status() const
{
    return static_cast<ConnectionStatus>(natsConnection_Status(_connection));
}

void Connection::setOption(Option option, const OptionValue& val)
{
    _options.set(option, val);
}

void NatsMq::Connection::connect(const Urls& urls)
{
    if (urls.empty())
        return;

    auto natsOptions = _options.rawOptions();
    auto urlPointers = createArrayPointersToElements(urls);

    exceptionIfError(natsOptions_SetServers(natsOptions, urlPointers.data(), static_cast<int>(urlPointers.size())));

    stateChanged(ConnectionStatus::Connecting);
    exceptionIfError(natsConnection_Connect(&_connection, natsOptions));
    stateChanged(ConnectionStatus::Connected);
}

void Connection::disconnect() const
{
    if (natsConnection_IsClosed(_connection))
        destroyConnection();
    else
        destroyConnectionWithWait();
}

natsConnection* NatsMq::Connection::rawConnection() const
{
    return _connection;
}

int Connection::registerConnectionCallback(ConnectionStateCb&& cb)
{
    _connectionCallbacks.push_back(std::move(cb));
    return static_cast<int>(_connectionCallbacks.size()) - 1;
}

int Connection::registerErrorCallback(ErrorCb&& cb)
{
    _errorCallbacks.push_back(std::move(cb));
    return static_cast<int>(_errorCallbacks.size()) - 1;
}

void Connection::unregisterConnectionCallback(int idx)
{
    _connectionCallbacks.erase(_connectionCallbacks.begin() + idx);
}

void Connection::unregisterErrorCallback(int idx)
{
    _errorCallbacks.erase(_errorCallbacks.begin() + idx);
}

void NatsMq::Connection::setConnectionHandlers(natsOptions* options)
{
    auto statusChangedCb = [](natsConnection* nc, void* closure) {
        const auto connection = reinterpret_cast<Connection*>(closure);
        const auto status     = static_cast<ConnectionStatus>(nc->status);
        connection->stateChanged(status);
    };

    auto closedCb = [](natsConnection* /*nc*/, void* closure) {
        const auto connection = reinterpret_cast<Connection*>(closure);
        connection->_mutex.unlock();
    };

    natsOptions_SetClosedCB(options, closedCb, this);
    natsOptions_SetDisconnectedCB(options, statusChangedCb, this);
    natsOptions_SetReconnectedCB(options, statusChangedCb, this);
}

void NatsMq::Connection::setErrorHandler(natsOptions* options)
{
    auto cb = [](natsConnection* /*nc*/, natsSubscription* /*subscription*/, natsStatus err, void* closure) {
        const auto connection = reinterpret_cast<Connection*>(closure);
        connection->errorOccured(static_cast<Status>(err), natsStatus_GetText(err));
    };

    natsOptions_SetErrorHandler(options, cb, this);
}

void NatsMq::Connection::destroyConnection() const
{
    natsConnection_Destroy(_connection);
}

void NatsMq::Connection::destroyConnectionWithWait() const
{
    _mutex.lock();
    destroyConnection();
    std::unique_lock<std::mutex> lc(_mutex);
}

void Connection::stateChanged(ConnectionStatus state) const
{
    for (auto&& cb : _connectionCallbacks)
        cb(state);
}

void Connection::errorOccured(NatsMq::Status status, const std::string& text) const
{
    for (auto&& cb : _errorCallbacks)
        cb(status, text);
}
