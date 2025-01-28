#include "JetStream.h"

#include "Exceptions.h"
#include "KeyValueStore.h"
#include "Message.h"
#include "MessageManager.h"
#include "ObjectStore.h"
#include "PullSubscription.h"
#include "Stream.h"
#include "Subscription.h"
#include "SyncSubscription.h"
#include "js/Context.h"
#include "js/KeyValueStorePrivate.h"
#include "js/MessageManagerPrivate.h"
#include "js/ObjectStorePrivate.h"
#include "js/Publisher.h"
#include "js/PullSubscriptionPrivate.h"
#include "js/StreamPrivate.h"
#include "js/SubscriptionPrivate.h"
#include "js/SyncSubscriptionPrivate.h"

using namespace NatsMq;

namespace
{
    using PrivateStreamPtr = std::unique_ptr<Js::StreamPrivate>;
}

JetStream::JetStream(std::shared_ptr<Connection> connection, std::unique_ptr<Js::Context> context)
    : _connection(connection)
    , _context(std::move(context))
{
}

JetStream::~JetStream() = default;

JetStream::JetStream(JetStream&&) = default;

JetStream& JetStream::operator=(JetStream&&) = default;

Js::Stream* JetStream::getOrCreateStream(const Js::StreamConfig& config) const
{
    PrivateStreamPtr privateStream(new Js::StreamPrivate(_context->rawContext(), config.name));
    if (!privateStream->exists())
        privateStream->create(config);

    return new Js::Stream(privateStream.release());
}

Js::Stream* JetStream::getStream(const std::string& name) const
{
    if (Js::StreamPrivate::exists(_context->rawContext(), name))
        return new Js::Stream(new Js::StreamPrivate(_context->rawContext(), name));

    jsExceptionIfError(NatsMq::Status::NotFound);
    return nullptr;
}

std::vector<Js::StreamInfo> JetStream::streamsInfo() const
{
    return Js::StreamPrivate::infos(_context->rawContext());
}

std::vector<std::string> JetStream::streamNames() const
{
    return Js::StreamPrivate::names(_context->rawContext());
}

Js::KeyValueStore* JetStream::getOrCreateKeyValueStore(const Js::KeyValue::Config& config) const
{
    return new Js::KeyValueStore(new Js::KeyValueStorePrivate(_context->rawContext(), config));
}

Js::KeyValueStore* JetStream::getKeyValueStore(const std::string& bucket) const
{
    if (Js::KeyValueStorePrivate::exists(_context->rawContext(), bucket))
    {
        Js::KeyValue::Config config;
        config.bucket = bucket;
        return new Js::KeyValueStore(new Js::KeyValueStorePrivate(_context->rawContext(), config));
    }

    jsExceptionIfError(NatsMq::Status::NotFound);
    return nullptr;
}

Js::ObjectStore* JetStream::getOrCreateObjectStore(const Js::ObjectStoreConfig& config) const
{
    return new Js::ObjectStore(new Js::ObjectStorePrivate(_context->rawContext(), config));
}

Js::ObjectStore* JetStream::getObjectStore(const std::string& bucket) const
{
    if (Js::ObjectStorePrivate::exists(_context->rawContext(), bucket))
    {
        Js::ObjectStoreConfig config;
        config.bucket = bucket;
        return getOrCreateObjectStore(config);
    }

    jsExceptionIfError(NatsMq::Status::NotFound);
    return nullptr;
}

Js::MessageManager* JetStream::messageManager() const
{
    return new Js::MessageManager(new Js::MessageManagerPrivate(_context->rawContext()));
}

Js::PublishAck JetStream::publish(Message msg, int64_t timeoutMs) const
{
    Js::PublishOptions options;
    options.timeout = timeoutMs;
    return publish(std::move(msg), std::move(options));
}

Js::PublishAck JetStream::publish(Message msg, Js::PublishOptions options) const
{
    Js::Publisher publisher(_context->rawContext());
    return publisher.publish(std::move(msg), std::move(options));
}

void JetStream::apublish(Message msg, int64_t timeoutMs) const
{
    Js::PublishOptions options;
    options.timeout = timeoutMs;
    apublish(std::move(msg), std::move(options));
}

void JetStream::apublish(Message msg, Js::PublishOptions options) const
{
    Js::Publisher publisher(_context->rawContext());
    publisher.apublish(std::move(msg), std::move(options));
}

void JetStream::registerAsyncPublishErrorHandler(Js::PublishErrorCb handler)
{
    _context->registerAsyncPublishErrorHandler(std::move(handler));
}

void JetStream::waitAsyncPublishComplete(int64_t timeoutMs) const
{
    Js::Publisher publisher(_context->rawContext());
    publisher.waitAsyncPublishComplete(timeoutMs);
}

std::vector<Message> JetStream::pendingMessages() const
{
    Js::Publisher publisher(_context->rawContext());
    return publisher.asyncPendingMessages();
}

Js::Subscription* JetStream::subscribe(const std::string& subject, const std::string& stream, JsSubscriptionCb cb) const
{
    Js::SubscriptionOptions options;
    options.stream = stream;
    return subscribe(subject, options, std::move(cb));
}

Js::Subscription* JetStream::subscribe(const std::string& subject, const Js::SubscriptionOptions& options, JsSubscriptionCb cb) const
{
    auto impl = new Js::SubscriptionPrivate(_context->rawContext());
    impl->registerListener(subject, options, std::move(cb));
    return new Js::Subscription(impl);
}

Js::SyncSubscription* JetStream::syncSubscribe(const std::string& subject, const std::string& stream)
{
    Js::SubscriptionOptions options;
    options.stream = stream;
    return syncSubscribe(subject, options);
}

Js::SyncSubscription* JetStream::syncSubscribe(const std::string& subject, const Js::SubscriptionOptions& options)
{
    return new Js::SyncSubscription(new Js::SyncSubscriptionPrivate(_context->rawContext(), subject, options));
}

Js::PullSubscription* JetStream::pullSubscribe(const std::string& subject, const std::string& stream)
{
    Js::SubscriptionOptions options;
    options.stream = stream;
    return pullSubscribe(subject, options);
}

Js::PullSubscription* JetStream::pullSubscribe(const std::string& subject, const Js::SubscriptionOptions& options)
{
    return new Js::PullSubscription(new Js::PullSubscriptionPrivate(_context->rawContext(), subject, options));
}
