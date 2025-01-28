#pragma once

#include <future>
#include <memory>
#include <vector>

#include "Entities.h"
#include "Export.h"
#include "Subscription.h"
#include "SyncSubscription.h"

// TODO cnatc managment: thread pool size and etc

namespace NatsMq
{
    class JetStream;
    class Connection;

    class NATSMQ_EXPORT Client
    {
    public:
        using Urls = std::vector<std::string>;

        static Client* create();

        Client(Connection*);

        ~Client();

        Client(const Client&);

        Client(Client&&);

        Client& operator=(const Client&);

        Client& operator=(Client&&);

        //! Get connection status
        ConnectionStatus connectionStatus() const;

        //! Connect to hosts
        void connect(const Urls& hosts, const ConnectionOptions& options = {}) const;

        //! Disconnect from hosts
        void disconnect() const;

        //! Check hosts access
        bool ping(int timeout) const noexcept;

        //! Get connection statistics
        IOStatistic statistics() const;

        //! Publish message
        void publish(Message msg) const;

        //! Request data. Request data. If there is no responder, an exception will be thrown
        Message request(Message msg, uint64_t timeoutMs = 2000) const;

        //! Same as request but async
        std::future<Message> arequest(Message msg, uint64_t timeoutMs = 2000) const;

        //! Create subscribtion
        Subscription* subscribe(const std::string& subject, SubscriptionCb cb) const;

        //! Similar as subscribe(subject, callback), but it's temporary subscribtion
        //! If no message is received by the given timeout (in milliseconds), the message handler is invoked with a empty message.
        Subscription* subscribe(const std::string& subject, int64_t timeoutMs, SubscriptionCb cb) const;

        //! Create queue subscribtion. If timeout > 0 then create temporary subscription.
        //! All subscribers with the same queue name will form the queue group and only one member of the group will be selected to receive any given message
        Subscription* subscribe(const std::string& subject, const std::string& queueGroup, SubscriptionCb cb) const;

        //! Similar as subscribe(subject, queue, callback), but it's temporary subscribtion.
        //! If no message is received by the given timeout (in milliseconds), the message handler is invoked with a empty message.
        Subscription* subscribe(const std::string& subject, const std::string& queueGroup, int64_t timeoutMs, SubscriptionCb cb) const;

        //! Create a synchronous subscription that can be polled via call next() for message recive
        SyncSubscription* syncSubscribe(const std::string& subject, const std::string& queueGroup = {}) const;

        //! Create jetstream
        JetStream* jetstream(const Js::Options& options = {}) const;

        //! Called all times when connection status changed
        int registerConnectionCallback(ConnectionStateCb);

        //! Remove registred callback
        void unregisterConnectionCallback(int idx) const;

        //! Called all times when error occured
        int registerErrorCallback(ErrorCb);

        //! Remove registred callback
        void unregisterErrorCallback(int idx) const;

    private:
        std::shared_ptr<Connection> _connection;
    };
}
