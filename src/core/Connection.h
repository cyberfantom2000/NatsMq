#pragma once

#include <nats.h>

#include <memory>
#include <mutex>
#include <vector>

#include "Entities.h"

namespace NatsMq
{
    class Connection
    {
    public:
        using Urls              = std::vector<std::string>;
        using NatsConnectionPtr = std::unique_ptr<natsConnection, decltype(&natsConnection_Destroy)>;
        using NatsOptionsPtr    = std::unique_ptr<natsOptions, decltype(&natsOptions_Destroy)>;

        Connection();

        ~Connection();

        ConnectionStatus status() const;

        void connect(const Urls& hosts, const ConnectionOptions& options);

        void disconnect();

        bool ping(int timeout) const noexcept;

        IOStatistic statistics() const;

        int registerConnectionCallback(ConnectionStateCb cb);

        int registerErrorCallback(ErrorCb cb);

        void unregisterConnectionCallback(int idx);

        void unregisterErrorCallback(int idx);

        natsConnection* rawConnection() const;

    private:
        void setConnectionHandlers(natsOptions* options);

        void setErrorHandler(natsOptions* options);

        void stateChanged(NatsMq::ConnectionStatus state) const;

        void errorOccured(NatsMq::Status status, const std::string& text) const;

    private:
        std::vector<ConnectionStateCb> _connectionCallbacks;
        std::vector<ErrorCb>           _errorCallbacks;

        NatsConnectionPtr _connection;
        NatsOptionsPtr    _options;
    };
}
