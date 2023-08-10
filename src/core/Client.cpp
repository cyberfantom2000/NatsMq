#include "Client.h"

#include "Connection.h"
#include "JetStream.h"
#include "MessageImpl.h"
#include "Publisher.h"
#include "Request.h"
#include "Statistics.h"
#include "SubscriptionImpl.h"
#include "Utils.h"
#include "versioncontrol.h"

namespace
{
    using NatsSubPtr = std::unique_ptr<natsSubscription, decltype(&natsSubscription_Destroy)>;
    using NatsMsgPtr = std::unique_ptr<natsMsg, decltype(&natsMsg_Destroy)>;
}

void NatsMq::Client::setThreadPoolSize(int count)
{
    configurePoolSize(count);
}

NatsMq::Client* NatsMq::Client::create()
{
    const auto connection = new Connection();
    return new Client(connection);
}

NatsMq::Client::Client(Connection* connection)
    : _connection(connection)
{
    staticCheckEnumIntegrity();
}

NatsMq::Client::~Client() = default;

NatsMq::ConnectionStatus NatsMq::Client::connectionStatus() const
{
    return _connection->status();
}

void NatsMq::Client::setOption(Option option, const OptionValue& val)
{
    _connection->setOption(option, val);
}

void NatsMq::Client::connect(const Urls& hosts)
{
    if (isConnected())
        disconnect();

    _connection->connect(hosts);
}

void NatsMq::Client::disconnect()
{
    _connection->disconnect();
}

bool NatsMq::Client::pingServer(int timeout) noexcept
{
    return makePing(_connection->rawConnection(), timeout);
}

NatsMq::IOStatistic NatsMq::Client::statistics()
{
    return Statistics::get(_connection->rawConnection());
}

void NatsMq::Client::publish(const Message& msg)
{
    Publisher publisher(_connection->rawConnection());
    publisher.puslish(msg);
}

NatsMq::IncomingMessage NatsMq::Client::request(const Message& msg, uint64_t timeoutMs)
{
    Request request(_connection->rawConnection());
    return request.make(msg, timeoutMs);
}

std::future<NatsMq::IncomingMessage> NatsMq::Client::asyncReuest(const Message& msg, uint64_t timeoutMs)
{
    Request request(_connection->rawConnection());
    return request.asyncMake(msg, timeoutMs);
}

NatsMq::Subscription NatsMq::Client::subscribe(const std::string& subject)
{
    auto privateSub = SubscriptionImpl::create(_connection->rawConnection(), subject);
    return Subscription(privateSub);
}

NatsMq::Subscription NatsMq::Client::subscribe(const std::string& subject, const std::string& queueGroup)
{
    auto privateSub = SubscriptionImpl::create(_connection->rawConnection(), subject, queueGroup);
    return Subscription(privateSub);
}

NatsMq::JetStream* NatsMq::Client::createJetStream(const JsOptions& options)
{
    return JetStream::configureAndCreate(_connection, options);
}

int NatsMq::Client::registerConnectionCallback(const ConnectionStateCb& cb)
{
    return _connection->registerConnectionCallback(ConnectionStateCb(cb));
}

int NatsMq::Client::registerConnectionCallback(ConnectionStateCb&& cb)
{
    return _connection->registerConnectionCallback(std::move(cb));
}

void NatsMq::Client::unregisterConnectionCallback(int idx)
{
    _connection->unregisterConnectionCallback(idx);
}

int NatsMq::Client::registerErrorCallback(const ErrorCb& cb)
{
    return _connection->registerErrorCallback(ErrorCb(cb));
}

int NatsMq::Client::registerErrorCallback(ErrorCb&& cb)
{
    return _connection->registerErrorCallback(std::move(cb));
}

void NatsMq::Client::unregisterErrorCallback(int idx)
{
    _connection->unregisterErrorCallback(idx);
}

bool NatsMq::Client::isConnected() const
{
    return connectionStatus() == ConnectionStatus::Connected;
}
