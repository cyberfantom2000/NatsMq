#include <Client.h>
#include <Exceptions.h>
#include <JetStream.h>
#include <KeyValueStore.h>
#include <Message.h>
#include <Stream.h>
#include <gtest/gtest.h>

#include "helpers.h"
#include "utilitys.h"

using namespace Tests;
using namespace NatsMq;

namespace
{
    Js::KeyValue::Config createKVConfig(const std::string& bucket)
    {
        Js::KeyValue::Config config;
        config.bucket = bucket;
        return config;
    }

    void deleteStore(Js::KeyValueStore* store)
    {
        if (store)
            store->deleteStore();
        delete store;
    };

    using StorePtr = std::unique_ptr<Js::KeyValueStore, decltype(&deleteStore)>;
}

TEST(NatsMqKeyValueStoreTesting, create_kv)
{
    constexpr auto bucket{ "testStream33" };

    const auto js = createJetStream();

    const auto config = createKVConfig(bucket);

    StorePtr kv(js->getOrCreateKeyValueStore(config), &deleteStore);

    EXPECT_EQ(kv->bucket(), config.bucket);
}

TEST(NatsMqKeyValueStoreTesting, delete_kv)
{
    constexpr auto bucket{ "testStream33" };

    const auto js = createJetStream();

    const auto config = createKVConfig(bucket);

    std::unique_ptr<Js::KeyValueStore> kv(js->getOrCreateKeyValueStore(config));

    kv->deleteStore();

    EXPECT_THROW({ js->getKeyValueStore(bucket); }, NatsMq::JsException);
}

TEST(NatsMqKeyValueStoreTesting, info)
{
    constexpr auto bucket{ "testStream33" };

    const auto js = createJetStream();

    auto config    = createKVConfig(bucket);
    config.history = 10;

    StorePtr kv(js->getOrCreateKeyValueStore(config), &deleteStore);

    const auto info = kv->info();

    EXPECT_EQ(info.bucket, config.bucket);
    EXPECT_EQ(info.history, config.history);
}

TEST(NatsMqKeyValueStoreTesting, put_string)
{
    constexpr auto bucket{ "testStream33" };
    constexpr auto key{ "testKey" };
    constexpr auto value{ "testValue" };

    const auto js = createJetStream();

    auto config = createKVConfig(bucket);

    StorePtr kv(js->getOrCreateKeyValueStore(config), &deleteStore);
    kv->put(key, value);

    const auto keys = kv->keys();

    const auto it = std::find(keys.begin(), keys.end(), key);
    EXPECT_TRUE(it != keys.end());
}

TEST(NatsMqKeyValueStoreTesting, create_string)
{
    constexpr auto bucket{ "testStream33" };
    constexpr auto key{ "testKey" };
    constexpr auto value{ "testValue" };

    const auto js = createJetStream();

    auto config = createKVConfig(bucket);

    StorePtr kv(js->getOrCreateKeyValueStore(config), &deleteStore);
    kv->create(key, value);

    const auto keys = kv->keys();

    const auto it = std::find(keys.begin(), keys.end(), key);
    EXPECT_TRUE(it != keys.end());
}

TEST(NatsMqKeyValueStoreTesting, create_string_error)
{
    constexpr auto bucket{ "testStream33" };
    constexpr auto key{ "testKey" };
    constexpr auto value{ "testValue" };

    const auto js = createJetStream();

    auto config = createKVConfig(bucket);

    StorePtr kv(js->getOrCreateKeyValueStore(config), &deleteStore);
    kv->create(key, value);

    EXPECT_THROW({ kv->create(key, value); }, NatsMq::JsException);
}

TEST(NatsMqKeyValueStoreTesting, get_string)
{
    constexpr auto bucket{ "testStream33" };
    constexpr auto key{ "testKey" };
    constexpr auto value{ "testValue" };

    const auto js = createJetStream();

    auto config = createKVConfig(bucket);

    StorePtr kv(js->getOrCreateKeyValueStore(config), &deleteStore);
    kv->put(key, value);

    const auto val = kv->getString(key);

    EXPECT_EQ(val, value);
}

TEST(NatsMqKeyValueStoreTesting, put)
{
    constexpr auto       bucket{ "testStream33" };
    constexpr auto       key{ "testKey" };
    std::vector<uint8_t> value{ 0x3, 0x4, 0x5, 0xF };

    const auto js = createJetStream();

    auto config = createKVConfig(bucket);

    StorePtr kv(js->getOrCreateKeyValueStore(config), &deleteStore);
    kv->put(key, value);

    const auto keys = kv->keys();

    const auto it = std::find(keys.begin(), keys.end(), key);
    EXPECT_TRUE(it != keys.end());
}

TEST(NatsMqKeyValueStoreTesting, create)
{
    constexpr auto       bucket{ "testStream33" };
    constexpr auto       key{ "testKey" };
    std::vector<uint8_t> value{ 0x3, 0x4, 0x5, 0xF };

    const auto js = createJetStream();

    auto config = createKVConfig(bucket);

    StorePtr kv(js->getOrCreateKeyValueStore(config), &deleteStore);
    kv->create(key, value);

    const auto keys = kv->keys();

    const auto it = std::find(keys.begin(), keys.end(), key);
    EXPECT_TRUE(it != keys.end());
}

TEST(NatsMqKeyValueStoreTesting, get)
{
    constexpr auto       bucket{ "testStream33" };
    constexpr auto       key{ "testKey" };
    std::vector<uint8_t> value{ 0x3, 0x4, 0x5, 0xF };

    const auto js = createJetStream();

    auto config = createKVConfig(bucket);

    StorePtr kv(js->getOrCreateKeyValueStore(config), &deleteStore);
    kv->put(key, value);

    const auto val = kv->get(key);

    EXPECT_EQ(val.value, value);
}

TEST(NatsMqKeyValueStoreTesting, remove)
{
    constexpr auto bucket{ "testStream33" };
    constexpr auto key{ "testKey" };

    const auto js = createJetStream();

    auto config = createKVConfig(bucket);

    StorePtr kv(js->getOrCreateKeyValueStore(config), &deleteStore);
    kv->put(key, "test");

    kv->remove(key);

    const auto keys = kv->keys();

    const auto it = std::find(keys.begin(), keys.end(), key);
    EXPECT_TRUE(it == keys.end());
}

TEST(NatsMqKeyValueStoreTesting, purge)
{
    constexpr auto bucket{ "testStream33" };
    constexpr auto key{ "testKey" };

    const auto js = createJetStream();

    auto config = createKVConfig(bucket);

    StorePtr kv(js->getOrCreateKeyValueStore(config), &deleteStore);
    kv->put(key, "test");

    kv->purge(key);

    const auto keys = kv->keys();

    const auto it = std::find(keys.begin(), keys.end(), key);
    EXPECT_TRUE(it == keys.end());
}
