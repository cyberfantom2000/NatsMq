#pragma once

#include <Client.h>
#include <gtest/gtest.h>

#include <QProcess>

class QTcpServer;

namespace Tests
{
    constexpr auto python{ "py" };
    constexpr auto pyFile{ "SimpleNatsClient.py" };
    constexpr auto clientThreadPoolSize{ 1 };
    constexpr auto validNatsUrl{ "nats://localhost:4222" };

    struct SignalEmitCounter
    {
        SignalEmitCounter(NatsMq::Client* client, int expectStateChanged = -1, int expectErrors = -1);

        ~SignalEmitCounter();

        int connectionChangedCount;
        int errorOccuredCount;

    private:
        int _expectStateChanged;
        int _expectErrors;
    };

    struct SignalCatcher : public QObject
    {
        SignalCatcher() = default;
    };

    class CommandWaiter
    {
    public:
        CommandWaiter(const QString& host = "127.0.0.1", int port = 8888);

        ~CommandWaiter();

        bool wait(const QByteArray& cmd, int timeout);

    private:
        std::unique_ptr<QTcpServer> _server;
    };

    void stopAndDeleteProcess(QProcess* process);

    struct ProcessDescriptor
    {
        int         timeout;
        QString     app;
        QStringList args;
    };

    using ProcessUPtr = std::unique_ptr<QProcess, decltype(&stopAndDeleteProcess)>;

    ProcessUPtr createProcess();

    void startAndWaitStarted(QProcess* process, const ProcessDescriptor& descriptor);

    template <typename Func>
    decltype(auto) callAfterStart(QProcess* process, const ProcessDescriptor& descriptor, Func&& func)
    {
        startAndWaitStarted(process, descriptor);
        return func();
    }

    template <typename SubType>
    void waitSubscriptionData(SubType& sub, const NatsMq::ByteArray& data, int timeoutMs = 3000)
    {
        std::mutex              m;
        std::condition_variable cv;

        sub.registerListener([&data, &cv](auto&& msg) {
            cv.notify_one();
            EXPECT_EQ(data, msg.data);
        });

        std::unique_lock<std::mutex> lc(m);

        const auto status = cv.wait_for(lc, std::chrono::milliseconds(timeoutMs));
        if (status == std::cv_status::timeout)
            GTEST_FAIL() << "Subscription timeout";
    }
}
