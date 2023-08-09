#include "TestUtils.h"

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
    if (_expectStateChanged >= 0)
        EXPECT_EQ(connectionChangedCount, _expectStateChanged);
    if (_expectErrors >= 0)
        EXPECT_EQ(errorOccuredCount, _expectErrors);
}

void Tests::stopAndDeleteProcess(QProcess* process)
{
    if (!process->waitForFinished(2000))
        process->kill();
    delete process;
}

Tests::ProcessUPtr Tests::createProcess()
{
    ProcessUPtr process(new QProcess, &stopAndDeleteProcess);
    return process;
}

void Tests::startAndWaitStarted(QProcess* process, const ProcessDescriptor& descriptor)
{
    CommandWaiter waiter;

    process->start(descriptor.app, descriptor.args);

    if (!waiter.wait("ready", descriptor.timeout))
        GTEST_FAIL() << "Timeout for ready command has expired.";
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
