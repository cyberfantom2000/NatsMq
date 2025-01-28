#include "Connection.h"

#include <opts.h>

#include "Entities.h"
#include "Exceptions.h"
#include "private/utils.h"

using namespace NatsMq;

namespace
{

    using NatsStatsPtr = std::unique_ptr<natsStatistics, decltype(&natsStatistics_Destroy)>;

    natsOptions* createNatsOptions(const ConnectionOptions& options)
    {
        natsOptions* opts;
        natsOptions_Create(&opts);
        NatsMq::exceptionIfError(natsOptions_SetNoRandomize(opts, !options.randomize));
        NatsMq::exceptionIfError(natsOptions_SetTimeout(opts, options.timeout));
        NatsMq::exceptionIfError(natsOptions_SetVerbose(opts, options.verbose));
        NatsMq::exceptionIfError(natsOptions_SetPedantic(opts, options.pedanic));
        NatsMq::exceptionIfError(natsOptions_SetPingInterval(opts, options.pingInterval));
        NatsMq::exceptionIfError(natsOptions_SetMaxPingsOut(opts, options.maxPingsOut));
        NatsMq::exceptionIfError(natsOptions_SetAllowReconnect(opts, options.allowRecconect));
        NatsMq::exceptionIfError(natsOptions_SetMaxReconnect(opts, options.maxReconnect));
        NatsMq::exceptionIfError(natsOptions_SetReconnectWait(opts, options.reconnectWait));
        NatsMq::exceptionIfError(natsOptions_SetReconnectBufSize(opts, options.reconnectBufferSize));
        NatsMq::exceptionIfError(natsOptions_SetMaxPendingMsgs(opts, options.maxPendingMessages));
        NatsMq::exceptionIfError(natsOptions_SetNoEcho(opts, !options.echo));
        NatsMq::exceptionIfError(natsOptions_SetRetryOnFailedConnect(opts, options.retryOnFailedConnect, nullptr, nullptr));
        NatsMq::exceptionIfError(natsOptions_SetSendAsap(opts, options.sendAsap));
        NatsMq::exceptionIfError(natsOptions_UseGlobalMessageDelivery(opts, options.useGlobalMsgDelivery));
        NatsMq::exceptionIfError(natsOptions_SetFailRequestsOnDisconnect(opts, options.failRequestOnDisconnect));
        return opts;
    }

}

NatsMq::Connection::Connection()
    : _connection(nullptr, &natsConnection_Destroy)
    , _options(nullptr, &natsOptions_Destroy)
{
}

Connection::~Connection()
{
}

ConnectionStatus Connection::status() const
{
    return static_cast<ConnectionStatus>(natsConnection_Status(_connection.get()));
}

void Connection::connect(const Urls& hosts, const ConnectionOptions& options)
{
    if (hosts.empty())
        return;

    _options.reset(createNatsOptions(options));

    auto natsOptions = _options.get();

    setErrorHandler(natsOptions);
    setConnectionHandlers(natsOptions);

    auto urlPointers = createArrayPointersToElements(hosts);

    exceptionIfError(natsOptions_SetServers(natsOptions, urlPointers.data(), static_cast<int>(urlPointers.size())));

    stateChanged(ConnectionStatus::Connecting);
    natsConnection* connection;
    exceptionIfError(natsConnection_Connect(&connection, natsOptions));
    _connection.reset(connection);
    stateChanged(ConnectionStatus::Connected);
}

void Connection::disconnect()
{
    _connection.reset();
    _options.reset();
}

bool Connection::ping(int timeout) const noexcept
{
    const auto status = static_cast<Status>(natsConnection_FlushTimeout(_connection.get(), timeout));
    return status == Status::Ok;
}

IOStatistic Connection::statistics() const
{
    natsStatistics* natsStats;
    exceptionIfError(natsStatistics_Create(&natsStats));

    NatsStatsPtr stats(natsStats, &natsStatistics_Destroy);
    exceptionIfError(natsConnection_GetStats(_connection.get(), natsStats));

    IOStatistic out;

    exceptionIfError(natsStatistics_GetCounts(natsStats, &out.inMsgs, &out.inBytes,
                                              &out.outMsgs, &out.outBytes, &out.reconnected));

    return out;
}

int Connection::registerConnectionCallback(ConnectionStateCb cb)
{
    _connectionCallbacks.push_back(std::move(cb));
    return static_cast<int>(_connectionCallbacks.size()) - 1;
}

int Connection::registerErrorCallback(ErrorCb cb)
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

natsConnection* NatsMq::Connection::rawConnection() const
{
    return _connection.get();
}

void NatsMq::Connection::setConnectionHandlers(natsOptions* options)
{
    auto statusChangedCb = [](natsConnection* nc, void* closure) {
        const auto connection = reinterpret_cast<Connection*>(closure);
        const auto status     = static_cast<ConnectionStatus>(nc->status);
        connection->stateChanged(status);
    };

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
