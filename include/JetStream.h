#pragma once

#include <memory>

#include "Entities.h"
#include "Export.h"

namespace NatsMq
{
    class Connection;

    namespace Js
    {
        class Context;
        class Stream;
        class MessageManager;
        class KeyValueStore;
        class ObjectStore;
        class Subscription;
        class SyncSubscription;
        class PullSubscription;
    }

    class NATSMQ_EXPORT JetStream
    {
    public:
        JetStream(std::shared_ptr<Connection> connection, std::unique_ptr<Js::Context> context);

        ~JetStream();

        JetStream(JetStream&&);

        JetStream& operator=(JetStream&&);

        //! Get or create new stream
        Js::Stream* getOrCreateStream(const Js::StreamConfig& config) const;

        //! Get stream, exception if stream does not exists
        Js::Stream* getStream(const std::string& name) const;

        //! Get information about all streams
        std::vector<Js::StreamInfo> streamsInfo() const;

        //! Get all stream names
        std::vector<std::string> streamNames() const;

        //! Get or create key-value store
        Js::KeyValueStore* getOrCreateKeyValueStore(const Js::KeyValue::Config& config) const;

        //! Get key-value store, exception if store does not exists
        Js::KeyValueStore* getKeyValueStore(const std::string& bucket) const;

        //! Get or create object store
        Js::ObjectStore* getOrCreateObjectStore(const Js::ObjectStoreConfig& config) const;

        //! Get object store, exception if store does not exists
        Js::ObjectStore* getObjectStore(const std::string& bucket) const;

        //! Сreate message manager to work with a raw streams
        Js::MessageManager* messageManager() const;

        //! Publish message
        Js::PublishAck publish(Message msg, int64_t timeoutMs = 2000) const;

        //! Publish message
        Js::PublishAck publish(Message msg, Js::PublishOptions options) const;

        //! Async publish message
        void apublish(Message msg, int64_t timeoutMs = 2000) const;

        //! Async publish message
        void apublish(Message msg, Js::PublishOptions options) const;

        //! Register a handler to be called when an async publish error occurs.
        void registerAsyncPublishErrorHandler(Js::PublishErrorCb handler);

        //! Wait until all asynchronously published messages
        void waitAsyncPublishComplete(int64_t timeoutMs = 2000) const;

        //! Return ownership all async published messages for which no acknowledgment have been received yet.
        std::vector<Message> pendingMessages() const;

        //! Сreate a subscription in which you can register a listener and receive auto notifications
        Js::Subscription* subscribe(const std::string& stream, const std::string& subject, JsSubscriptionCb cb) const;

        //! Сreate a subscription in which you can register a listener and receive auto notifications
        Js::Subscription* subscribe(const std::string& subject, const Js::SubscriptionOptions& options, JsSubscriptionCb) const;

        //! Сreate a subscription in which you must request notifications manually
        Js::SyncSubscription* syncSubscribe(const std::string& subject, const std::string& stream);

        //! Сreate a subscription in which you must request notifications manually
        Js::SyncSubscription* syncSubscribe(const std::string& subject, const Js::SubscriptionOptions& options);

        //! Create a pull based subscription. Consumer that does not have a delivery subject,
        //! that is the library has to request for the messages to be delivered as needed from the server.
        Js::PullSubscription* pullSubscribe(const std::string& subject, const std::string& stream);

        //! Create a pull based subscription. Consumer that does not have a delivery subject,
        //! that is the library has to request for the messages to be delivered as needed from the server.
        Js::PullSubscription* pullSubscribe(const std::string& subject, const Js::SubscriptionOptions& options);

    private:
        std::shared_ptr<Connection>  _connection;
        std::unique_ptr<Js::Context> _context;
    };
};
