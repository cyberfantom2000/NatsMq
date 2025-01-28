#include "Client.h"

#include "JetStream.h"
#include "Message.h"
#include "core/Connection.h"
#include "core/Publisher.h"
#include "core/Requestor.h"
#include "core/SubscriptionPrivate.h"
#include "core/SyncSubscriptionPrivate.h"
#include "js/Context.h"
#include "private/versioncontrol.h"

using namespace NatsMq;

Client* Client::create()
{
    const auto connection = new Connection();
    return new Client(connection);
}

Client::Client(Connection* connection)
    : _connection(connection)
{
    staticCheckEnumIntegrity();
}

Client::~Client() = default;

Client::Client(const Client&) = default;

Client::Client(Client&&) = default;

Client& Client::operator=(const Client&) = default;

Client& Client::operator=(Client&&) = default;

ConnectionStatus Client::connectionStatus() const
{
    return _connection->status();
}

void Client::connect(const Urls& hosts, const ConnectionOptions& options) const
{
    if (connectionStatus() == ConnectionStatus::Connected)
        disconnect();

    _connection->connect(hosts, options);
}

void Client::disconnect() const
{
    _connection->disconnect();
}

bool Client::ping(int timeout) const noexcept
{
    return _connection->ping(timeout);
}

IOStatistic Client::statistics() const
{
    return _connection->statistics();
}

void Client::publish(Message msg) const
{
    Publisher publisher(_connection->rawConnection());
    publisher.publish(std::move(msg));
}

Message Client::request(Message msg, uint64_t timeoutMs) const
{
    Requestor requestor(_connection->rawConnection());
    return requestor.request(std::move(msg), timeoutMs);
}

std::future<Message> Client::arequest(Message msg, uint64_t timeoutMs) const
{
    Requestor requestor(_connection->rawConnection());
    return requestor.arequest(std::move(msg), timeoutMs);
}

Subscription* Client::subscribe(const std::string& subject, SubscriptionCb cb) const
{
    auto impl = new SubscriptionPrivate(_connection->rawConnection(), subject);
    impl->registerListener(std::move(cb));
    return new Subscription(impl);
}

Subscription* Client::subscribe(const std::string& subject, int64_t timeoutMs, SubscriptionCb cb) const
{
    auto impl = new SubscriptionPrivate(_connection->rawConnection(), subject, timeoutMs);
    impl->registerListener(std::move(cb));
    return new Subscription(impl);
}

Subscription* Client::subscribe(const std::string& subject, const std::string& queue, SubscriptionCb cb) const
{
    auto impl = new SubscriptionPrivate(_connection->rawConnection(), subject, queue);
    impl->registerListener(std::move(cb));
    return new Subscription(impl);
}

Subscription* Client::subscribe(const std::string& subject, const std::string& queue, int64_t timeoutMs, SubscriptionCb cb) const
{
    auto impl = new SubscriptionPrivate(_connection->rawConnection(), subject, queue, timeoutMs);
    impl->registerListener(std::move(cb));
    return new Subscription(impl);
}

SyncSubscription* Client::syncSubscribe(const std::string& subject, const std::string& queue) const
{
    return new SyncSubscription(queue.empty() ? new SyncSubscriptionPrivate(_connection->rawConnection(), subject) : new SyncSubscriptionPrivate(_connection->rawConnection(), subject, queue));
}

JetStream* Client::jetstream(const Js::Options& options) const
{
    auto context = std::make_unique<Js::Context>(_connection->rawConnection(), options);
    return new JetStream(_connection, std::move(context));
}

int Client::registerConnectionCallback(ConnectionStateCb cb)
{
    return _connection->registerConnectionCallback(std::move(cb));
}

void Client::unregisterConnectionCallback(int idx) const
{
    _connection->unregisterConnectionCallback(idx);
}

int Client::registerErrorCallback(ErrorCb cb)
{
    return _connection->registerErrorCallback(std::move(cb));
}

void Client::unregisterErrorCallback(int idx) const
{
    _connection->unregisterErrorCallback(idx);
}
