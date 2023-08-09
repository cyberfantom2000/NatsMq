#include <gtest/gtest.h>

#include "Client.h"
#include "Exceptions.h"
#include "JetStream.h"
#include "TestUtils.h"

using namespace Tests;
using namespace NatsMq;

namespace
{
    constexpr auto streamName{ "test_stream" };
    constexpr auto subject{ "test_subject" };

    std::unique_ptr<JetStream> createJetStream()
    {
        const auto client = std::unique_ptr<Client>(Client::configureAndCreate(clientThreadPoolSize));
        client->connect({ validNatsUrl });
        return std::unique_ptr<JetStream>(client->createJetStream());
    }

    JsStreamConfig createConfigWithMemoryStorage(const std::string& name, const std::vector<std::string>& subjects)
    {
        JsStreamConfig config;
        config.name     = name;
        config.storage  = JsStreamConfig::Storage::Memory;
        config.subjects = subjects;
        return config;
    }

    template <typename Callback = void (*)(JsStream&)>
    void createStreamCallDeleteStream(JetStream* js, const JsStreamConfig& config, Callback&& cb)
    {
        auto stream = std::unique_ptr<JsStream>(js->getOrCreateStream(config));
        try
        {
            cb(stream.get());
            stream->remove();
        }
        catch (...)
        {
            stream->remove();
            throw;
        }
    }

    template <typename Callback>
    void createStoreCallDeleteStore(JetStream* js, const KeyValueConfig& config, Callback&& cb)
    {
        auto kv = std::unique_ptr<KeyValueStore>(js->getOrCreateStore(config));
        try
        {
            cb(kv.get());
            kv->deleteStore();
        }
        catch (...)
        {
            kv->deleteStore();
            throw;
        }
    }

    KeyValueConfig defautlKvConfig(const std::string& name = "test_kv")
    {
        KeyValueConfig c;
        c.name = name;
        return c;
    }
}

TEST(NatsMqJetStreamTesting, create_delete_stream)
{
    const auto js     = createJetStream();
    const auto config = createConfigWithMemoryStorage(streamName, { subject });

    const auto stream = js->getOrCreateStream(config);
    stream->remove();
}

TEST(NatsMqJetStreamTesting, delete_stream_error)
{
    const auto js = createJetStream();

    const auto stream = js->getStream(streamName);
    EXPECT_THROW({ stream->remove(); }, JsException);
}

TEST(NatsMqJetStreamTesting, purge_stream_error)
{
    const auto js = createJetStream();

    const auto stream = js->getStream(streamName);
    EXPECT_THROW({ stream->purge(); }, JsException);
}

TEST(NatsMqJetStreamTesting, purge_stream)
{
    const auto js     = createJetStream();
    const auto config = createConfigWithMemoryStorage(streamName, { subject });

    createStreamCallDeleteStream(js.get(), config, [](auto&& stream) { stream->purge(); });
}

TEST(NatsMqJetStreamTesting, update_stream_error)
{
    const auto js     = createJetStream();
    const auto config = createConfigWithMemoryStorage(streamName, { subject });

    auto stream = js->getStream(streamName);
    EXPECT_THROW({ stream->update(config); }, JsException);
}

TEST(NatsMqJetStreamTesting, update_stream)
{
    const auto js = createJetStream();

    auto config = createConfigWithMemoryStorage(streamName, { subject });

    auto cb = [&config](auto&& stream) {
        config.subjects = { "newSubj" };
        stream->update(config);
    };

    createStreamCallDeleteStream(js.get(), config, cb);
}

TEST(NatsMqJetStreamTesting, publish)
{
    const auto js     = createJetStream();
    const auto config = createConfigWithMemoryStorage(streamName, { subject });

    auto cb = [&js](auto&& /*stream*/) { js->publish(NatsMq::Message(subject, "data")); };
    createStreamCallDeleteStream(js.get(), config, cb);
}

TEST(NatsMqJetStreamTesting, subscribe)
{
    constexpr auto expectMsg{ "test_subscribe" };

    const auto js     = createJetStream();
    const auto config = createConfigWithMemoryStorage(streamName, { subject });

    auto cb = [&js](auto&& /*stream*/) {
        auto        sub = js->subscribe(streamName, subject);
        QStringList arguments{ pyFile, "--publish_subject", subject, "--publish_data", expectMsg };
        const auto  process = createProcess();
        process->start(python, arguments);
        waitSubscriptionData<NatsMq::JsSubscription>(sub, NatsMq::ByteArray(expectMsg));
    };

    createStreamCallDeleteStream(js.get(), config, cb);
}

TEST(NatsMqJetStreamTesting, pull_subscribe)
{
    constexpr auto expectMsg{ "test_subscribe" };

    const auto js     = createJetStream();
    const auto config = createConfigWithMemoryStorage(streamName, { subject });

    auto cb = [&js](auto&& /*stream*/) {
        const auto  sub = js->pullSubscribe(streamName, subject);
        QStringList arguments{ pyFile, "--publish_subject", subject, "--publish_data", expectMsg };
        const auto  process = createProcess();
        process->start(python, arguments);

        const auto msgs = sub.fetch();
        for (auto&& msg : msgs)
            EXPECT_EQ(msg.data, NatsMq::ByteArray(expectMsg));
    };

    createStreamCallDeleteStream(js.get(), config, cb);
}

TEST(NatsMqJetStreamTesting, key_value_get_error)
{
    const auto js     = createJetStream();
    const auto config = defautlKvConfig();

    EXPECT_THROW({ std::unique_ptr<KeyValueStore>(js->getStore(config)); }, Exception);
}

TEST(NatsMqJetStreamTesting, key_value_create_remove)
{
    const auto js     = createJetStream();
    const auto config = defautlKvConfig();

    std::unique_ptr<KeyValueStore> store(js->getOrCreateStore(config));
    store->deleteStore();
}

TEST(NatsMqJetStreamTesting, key_value_create_element)
{
    const auto js     = createJetStream();
    const auto config = defautlKvConfig();

    auto cb = [](auto&& kv) {
        constexpr auto key{ "test_key" };
        constexpr auto value{ "test_value" };

        kv->putElement(key, value);
        const auto keys = kv->keys();
        const auto it   = std::find(keys.cbegin(), keys.cend(), key);
        EXPECT_TRUE(it != keys.cend());
    };

    createStoreCallDeleteStore(js.get(), config, cb);
}

TEST(NatsMqJetStreamTesting, key_value_purge_element)
{
    const auto js     = createJetStream();
    const auto config = defautlKvConfig();

    auto cb = [](auto&& kv) {
        constexpr auto key{ "test_key" };
        kv->purgeElement(key);
        kv->removeElement(key);
    };

    createStoreCallDeleteStore(js.get(), config, cb);
}

TEST(NatsMqJetStreamTesting, key_value_store_name)
{
    const auto js     = createJetStream();
    const auto config = defautlKvConfig();

    createStoreCallDeleteStore(js.get(), config, [&config](auto&& kv) { EXPECT_EQ(kv->storeName(), config.name); });
}
