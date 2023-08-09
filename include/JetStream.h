#pragma once

#include <memory>

#include "JsKeyValue.h"
#include "JsStream.h"
#include "Message.h"
#include "Subscription.h"

namespace NatsMq
{
    class Connection;
    class Context;

    class JetStream
    {
    public:
        static JetStream* configureAndCreate(std::shared_ptr<Connection> connection, const JsOptions& options);

        ~JetStream();

        JsStream* getOrCreateStream(const JsStreamConfig& config) const;

        JsStream* getStream(const std::string& name) const;

        KeyValueStore* getOrCreateStore(const KeyValueConfig& config) const;

        KeyValueStore* getStore(const KeyValueConfig& config) const;

        //! Delete Key-Value store on NATS server
        void deleteStore(const std::string& name) const;

        JsPublishAck publish(const Message& msg, const JsPublishOptions& options) const;

        JsPublishAck publish(const Message& msg, int64_t timeout = 2000) const;

        //! Register a handler to be called when an async publish error occurs.
        void registerAsyncErrorHandler(const JsAsyncErrorCb& errorHandler) const;

        void registerAsyncErrorHandler(JsAsyncErrorCb&& errorHandler) const;

        JsPublishAck asyncPublish(const Message& msg, int64_t timeout = 2000) const;

        JsPublishAck asyncPublish(const Message& msg, const JsPublishOptions& options) const;

        //! Wait until all asynchronously published messages
        void waitAsyncPublishCompleted(int64_t timeout = -1) const;

        //! Return ownership all async published messages for which no acknowledgment have been received yet.
        std::vector<Message> getAsyncPendingMessages() const;

        //! Сreate a subscription in which you can register a listener and receive auto notifications
        JsSubscription subscribe(const std::string& stream, const std::string& subject, const std::string& consumer = {}) const;

        //! Сreate a subscription in which you must request notifications manually
        JsPullSubscription pullSubscribe(const std::string& stream, const std::string& subject, const std::string& consumer = {}) const;

    private:
        JetStream(std::shared_ptr<Connection> connection, std::unique_ptr<Context> context);

    private:
        std::shared_ptr<Connection> _connection;
        std::unique_ptr<Context>    _context;
    };
};
