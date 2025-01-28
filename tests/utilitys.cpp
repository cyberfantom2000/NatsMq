#include "utilitys.h"

#include <Message.h>

#include <QElapsedTimer>
#include <QTcpServer>
#include <QTcpSocket>

Tests::SignalEmitCounter::SignalEmitCounter(NatsMq::Client* client, int expectStateChanged, int expectErrors)
    : connectionChangedCount(0)
    , errorOccuredCount(0)
    , _expectStateChanged(expectStateChanged)
    , _expectErrors(expectErrors)
{
    client->registerConnectionCallback([this](const NatsMq::ConnectionStatus /*status*/) { ++connectionChangedCount; });
    client->registerErrorCallback([this](const NatsMq::Status /*status*/, const std::string& /*text*/) { ++errorOccuredCount; });
}

Tests::SignalEmitCounter::~SignalEmitCounter()
{
    if (_expectStateChanged >= 0){
        EXPECT_EQ(connectionChangedCount, _expectStateChanged);
    }
    if (_expectErrors >= 0){
        EXPECT_EQ(errorOccuredCount, _expectErrors);
    }
}

Tests::CommandWaiter::CommandWaiter(const QString& host, int port)
    : _server(new QTcpServer)
{
    if (!_server->listen(QHostAddress(host), static_cast<quint16>(port)))
        throw std::runtime_error(_server->errorString().toStdString());
}

Tests::CommandWaiter::~CommandWaiter()
{
    _server->close();
}

bool Tests::CommandWaiter::wait(const QByteArray& cmd, int timeout)
{
    if (!_server->waitForNewConnection(timeout))
        return false;

    auto socket = _server->nextPendingConnection();

    if (!socket->waitForReadyRead(timeout))
        return false;

    const auto recvMsg = socket->readAll();
    return recvMsg == cmd;
}
