#pragma once

#include <nats.h>

#include <memory>
#include <mutex>

#include "Entities.h"
#include "Options.h"
namespace NatsMq
{
    class Options;

    class Connection
    {
    public:
        using Urls = std::vector<std::string>;

        Connection();

        ~Connection();

        Connection(Connection&&) = default;

        Connection& operator=(Connection&&) = default;

        ConnectionStatus status() const;

        void setOption(Option option, const OptionValue& val);

        //! Нет возможности перенести в конструктор т.к. будет потеряна возможность оповещать о состояниях connecting и connected
        void connect(const Urls& hosts);

        void disconnect() const;

        natsConnection* rawConnection() const;

        int registerConnectionCallback(ConnectionStateCb&&);

        int registerErrorCallback(ErrorCb&&);

        void unregisterConnectionCallback(int idx);

        void unregisterErrorCallback(int idx);

    private:
        void setConnectionHandlers(natsOptions* options);

        void setErrorHandler(natsOptions* options);

        void destroyConnection() const;

        void destroyConnectionWithWait() const;

        void stateChanged(NatsMq::ConnectionStatus state) const;

        void errorOccured(NatsMq::Status status, const std::string& text) const;

    private:
        std::vector<ConnectionStateCb> _connectionCallbacks;
        std::vector<ErrorCb>           _errorCallbacks;

        Options         _options;
        natsConnection* _connection;

        mutable std::mutex _mutex;
    };
}
