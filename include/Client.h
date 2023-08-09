#pragma once

#include <future>
#include <memory>
#include <optional>

#include "Message.h"
#include "Subscription.h"

namespace NatsMq
{
    class JetStream;
    class Connection;

    class Client
    {
    public:
        using Urls = std::vector<std::string>;

        static Client* configureAndCreate(int clientThreadPoolSize = 1);

        Client(Connection*);

        ~Client();

        ConnectionStatus connectionStatus() const;

        void connect(const Urls& hosts, const std::optional<Options>& options = {});

        void disconnect();

        bool pingServer(int timeout) noexcept;

        IOStatistic statistics();

        void publish(const Message& msg);

        IncomingMessage request(const Message& msg, uint64_t timeoutMs = 2000);

        std::future<IncomingMessage> asyncReuest(const Message& msg, uint64_t timeoutMs = 2000);

        Subscription subscribe(const std::string& subject);

        Subscription subscribe(const std::string& subject, const std::string& queueGroup);

        JetStream* createJetStream(const JsOptions& options = {});

        //! Called all times when connection status changed
        int registerConnectionCallback(const ConnectionStateCb&);

        int registerConnectionCallback(ConnectionStateCb&&);

        void unregisterConnectionCallback(int idx);

        //! Called all times when error occured
        int registerErrorCallback(const ErrorCb&);

        int registerErrorCallback(ErrorCb&&);

        void unregisterErrorCallback(int idx);

    private:
        bool isConnected() const;

    private:
        std::shared_ptr<Connection> _connection;
    };
}
