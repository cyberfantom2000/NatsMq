#pragma once

#include <future>
#include <memory>
#include <optional>

#include "Message.h"
#include "Subscription.h"
#include "natsmq_export.h"

namespace NatsMq
{
    class JetStream;
    class Connection;

    class NATSMQ_EXPORT Client
    {
    public:
        using Urls = std::vector<std::string>;

        static void setThreadPoolSize(int count = 1);

        static Client* create();

        Client(Connection*);

        ~Client();

        Client(const Client&) = default;

        Client(Client&&) = default;

        Client& operator=(const Client&) = default;

        Client& operator=(Client&&) = default;

        ConnectionStatus connectionStatus() const;

        void setOption(Option option, const OptionValue& val);

        void connect(const Urls& hosts) const;

        void disconnect() const;

        bool pingServer(int timeout) const noexcept;

        IOStatistic statistics() const;

        void publish(const Message& msg) const;

        IncomingMessage request(const Message& msg, uint64_t timeoutMs = 2000) const;

        std::future<IncomingMessage> asyncReuest(const Message& msg, uint64_t timeoutMs = 2000) const;

        Subscription subscribe(const std::string& subject) const;

        Subscription subscribe(const std::string& subject, const std::string& queueGroup) const;

        JetStream* createJetStream(const JsOptions& options = {}) const;

        //! Called all times when connection status changed
        int registerConnectionCallback(const ConnectionStateCb&) const;

        int registerConnectionCallback(ConnectionStateCb&&) const;

        void unregisterConnectionCallback(int idx) const;

        //! Called all times when error occured
        int registerErrorCallback(const ErrorCb&) const;

        int registerErrorCallback(ErrorCb&&) const;

        void unregisterErrorCallback(int idx) const;

    private:
        bool isConnected() const;

    private:
        std::shared_ptr<Connection> _connection;
    };
}
