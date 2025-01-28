#include <Client.h>
#include <Message.h>
#include <Stream.h>
#include <Subscription.h>
#include <gtest/gtest.h>

#include <QObject>
#include <QProcess>
#include <QString>

class QTcpServer;

namespace Tests
{
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

    struct ProcessDescriptor
    {
        int         timeout;
        QString     app;
        QStringList args;
    };

    inline void stopAndDeleteProcess(QProcess* process)
    {
        if (!process->waitForFinished(2000))
            process->kill();
        delete process;
    }

    using ProcessUPtr = std::unique_ptr<QProcess, decltype(&stopAndDeleteProcess)>;

    inline ProcessUPtr createProcess()
    {
        ProcessUPtr process(new QProcess, &stopAndDeleteProcess);
        return process;
    }

    inline void startAndWaitStarted(QProcess* process, const ProcessDescriptor& descriptor)
    {
        CommandWaiter waiter;

        process->start(descriptor.app, descriptor.args);

        if (!waiter.wait("ready", descriptor.timeout))
            GTEST_FAIL() << "Timeout for ready command has expired.";
    }

    template <typename Func>
    decltype(auto) callAfterStart(QProcess* process, const ProcessDescriptor& descriptor, Func&& func)
    {
        startAndWaitStarted(process, descriptor);
        return func();
    }

    inline NatsMq::Message msgFromString(const std::string& subj, const std::string& data)
    {
        NatsMq::Message msg;
        msg.subject = subj;
        msg.data.insert(msg.data.end(), data.begin(), data.end());

        return msg;
    }
}
