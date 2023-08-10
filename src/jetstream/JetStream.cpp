#include "JetStream.h"

#include <future>

#include "Connection.h"
#include "JsContext.h"
#include "JsKeyValueImpl.h"
#include "JsPublisher.h"
#include "JsStreamImpl.h"
#include "JsSubscriptionImpl.h"

NatsMq::JetStream* NatsMq::JetStream::configureAndCreate(std::shared_ptr<Connection> connection, const JsOptions& options)
{
    jsOptions natsJsOptions;
    jsOptions_Init(&natsJsOptions);
    natsJsOptions.Domain = options.domain.c_str();
    natsJsOptions.Wait   = options.timeout;

    auto context = std::make_unique<Context>(connection->rawConnection(), &natsJsOptions);
    return new JetStream(connection, std::move(context));
}

NatsMq::JetStream::~JetStream() = default;

NatsMq::JetStream::JetStream(JetStream&&) = default;

NatsMq::JetStream& NatsMq::JetStream::operator=(JetStream&&) = default;

NatsMq::JsStream* NatsMq::JetStream::getOrCreateStream(const JsStreamConfig& config) const
{
    return new JsStream(JsStreamImpl::getOrCreate(_context->rawContext(), config));
}

NatsMq::JsStream* NatsMq::JetStream::getStream(const std::string& name) const
{
    JsStreamConfig config;
    config.name = name.c_str();
    return new JsStream(JsStreamImpl::get(_context->rawContext(), config));
}

NatsMq::KeyValueStore* NatsMq::JetStream::getOrCreateStore(const KeyValueConfig& config) const
{
    return new KeyValueStore(KeyValueStoreImpl::getOrCreate(_context->rawContext(), config));
}

NatsMq::KeyValueStore* NatsMq::JetStream::getStore(const KeyValueConfig& config) const
{
    return new KeyValueStore(KeyValueStoreImpl::get(_context->rawContext(), config));
}

NatsMq::JetStream::JetStream(std::shared_ptr<Connection> connection, std::unique_ptr<Context> context)
    : _connection(connection)
    , _context(std::move(context))
{
}

NatsMq::JsPublishAck NatsMq::JetStream::publish(const Message& msg, const JsPublishOptions& options) const
{
    JsPublisher publisher(_context->rawContext());
    return publisher.publish(msg, options);
}

NatsMq::JsPublishAck NatsMq::JetStream::publish(const Message& msg, int64_t timeout) const
{
    JsPublishOptions options;
    options.timeout = timeout;
    return publish(msg, options);
}

void NatsMq::JetStream::registerAsyncErrorHandler(const JsAsyncErrorCb& errorHandler) const
{
    _context->registerAsyncErrorHandler(JsAsyncErrorCb(errorHandler));
}

void NatsMq::JetStream::registerAsyncErrorHandler(JsAsyncErrorCb&& errorHandler) const
{
    _context->registerAsyncErrorHandler(std::move(errorHandler));
}

void NatsMq::JetStream::asyncPublish(const Message& msg, int64_t timeout) const
{
    JsPublishOptions options;
    options.timeout = timeout;
    asyncPublish(msg, options);
}

void NatsMq::JetStream::asyncPublish(const Message& msg, const JsPublishOptions& options) const
{
    JsPublisher publisher(_context->rawContext());
    publisher.asyncPublish(msg, options);
}

void NatsMq::JetStream::waitAsyncPublishCompleted(int64_t timeout) const
{
    JsPublisher publisher(_context->rawContext());
    publisher.waitPublishCompleted(timeout);
}

std::vector<NatsMq::Message> NatsMq::JetStream::getAsyncPendingMessages() const
{
    JsPublisher publisher(_context->rawContext());
    return publisher.getAsyncPendingMessages();
}

NatsMq::JsSubscription NatsMq::JetStream::subscribe(const std::string& stream, const std::string& subject, const std::string& consumer) const
{
    auto impl = JsSubscriptionImpl::create(_context->rawContext(), stream, subject, consumer);
    return JsSubscription(impl);
}

NatsMq::JsPullSubscription NatsMq::JetStream::pullSubscribe(const std::string& stream, const std::string& subject, const std::string& consumer) const
{
    auto impl = JsPullSubscriptionImpl::create(_context->rawContext(), stream, subject, consumer);
    return JsPullSubscription(impl);
}
