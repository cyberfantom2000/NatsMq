#include <Client.h>
#include <Exceptions.h>
#include <JetStream.h>
#include <Message.h>
#include <ObjectStore.h>
#include <Stream.h>
#include <gtest/gtest.h>

#include "helpers.h"
#include "utilitys.h"

using namespace Tests;
using namespace NatsMq;

namespace
{
    void deleteStore(Js::ObjectStore* store)
    {
        if (store)
            store->deleteStore();
        delete store;
    }

    using ObjectStorePtr = std::unique_ptr<Js::ObjectStore, decltype(&deleteStore)>;

    Js::ObjectStoreConfig createOSConfig(const std::string& bucket)
    {
        Js::ObjectStoreConfig config;
        config.bucket = bucket;
        config.maxAge = 60;
        return config;
    }

    void putObject(Js::ObjectStore* os, const std::string& name, std::vector<uint8_t> data)
    {
        Js::ObjectElement obj;
        obj.meta.name = name;
        obj.data      = std::move(data);

        os->put(obj);
    }
}

TEST(NatsMqObjectStoreTesting, create_os)
{
    constexpr auto bucket{ "testObjectStore" };

    const auto js = createJetStream();

    const auto config = createOSConfig(bucket);

    ObjectStorePtr os(js->getOrCreateObjectStore(config), &deleteStore);

    EXPECT_TRUE(os->storeExists());
}

TEST(NatsMqObjectStoreTesting, delete_os)
{
    constexpr auto bucket{ "testObjectStore" };

    const auto js = createJetStream();

    const auto config = createOSConfig(bucket);

    std::unique_ptr<Js::ObjectStore> os(js->getOrCreateObjectStore(config));

    os->deleteStore();

    EXPECT_FALSE(os->storeExists());
    EXPECT_THROW({ js->getObjectStore(bucket); }, NatsMq::JsException);
}

TEST(NatsMqObjectStoreTesting, config_os)
{
    constexpr auto bucket{ "testObjectStore" };

    const auto js = createJetStream();

    const auto config = createOSConfig(bucket);

    ObjectStorePtr os(js->getOrCreateObjectStore(config), &deleteStore);

    auto storeConfig = os->storeConfig();

    EXPECT_EQ(storeConfig.bucket, config.bucket);
    EXPECT_EQ(storeConfig.description, config.description);
    EXPECT_EQ(storeConfig.maxAge, config.maxAge);
    EXPECT_EQ(storeConfig.maxBytes, config.maxBytes);
    EXPECT_EQ(storeConfig.storage, config.storage);
}

TEST(NatsMqObjectStoreTesting, put)
{
    constexpr auto bucket{ "testObjectStore" };
    constexpr auto name{ "objectName" };

    std::vector<uint8_t> bigdata{ 0x4, 0x6, 0x7, 0x4, 0x6, 0x7 };

    const auto js = createJetStream();

    const auto config = createOSConfig(bucket);

    ObjectStorePtr os(js->getOrCreateObjectStore(config), &deleteStore);

    Js::ObjectElement obj;
    obj.meta.name = name;
    obj.data      = std::move(bigdata);

    os->put(obj);
}

TEST(NatsMqObjectStoreTesting, info)
{
    constexpr auto bucket{ "testObjectStore" };
    constexpr auto name{ "objectName" };

    std::vector<uint8_t> bigdata{ 0x4, 0x6, 0x7, 0x4, 0x6, 0x7 };

    const auto js = createJetStream();

    const auto config = createOSConfig(bucket);

    ObjectStorePtr os(js->getOrCreateObjectStore(config), &deleteStore);

    Js::ObjectElement obj;
    obj.meta.name = name;
    obj.data      = bigdata;

    os->put(obj);

    const auto info = os->info(obj.meta.name);
    EXPECT_EQ(info.name, obj.meta.name);
    EXPECT_EQ(info.size, bigdata.size());
    EXPECT_EQ(info.bucket, bucket);
}

TEST(NatsMqObjectStoreTesting, put_big_object)
{
    constexpr auto bucket{ "testObjectStore" };
    constexpr auto name{ "objectName" };

    constexpr auto objectChunks{ 3 };
    constexpr auto chunkSize{ 128ull * 1024ull }; // 128 kB

    std::vector<uint8_t> bigdata(chunkSize * objectChunks);
    std::iota(bigdata.begin(), bigdata.end(), 0);

    const auto js = createJetStream();

    const auto config = createOSConfig(bucket);

    ObjectStorePtr os(js->getOrCreateObjectStore(config), &deleteStore);

    Js::ObjectElement obj;
    obj.meta.name = name;
    obj.data      = std::move(bigdata);

    os->put(obj);
}

TEST(NatsMqObjectStoreTesting, get)
{
    constexpr auto bucket{ "testObjectStore" };
    constexpr auto name{ "objectName" };

    std::vector<uint8_t> bigdata{ 0x4, 0x6, 0x7, 0x4, 0x6, 0x7 };

    const auto js = createJetStream();

    const auto config = createOSConfig(bucket);

    ObjectStorePtr os(js->getOrCreateObjectStore(config), &deleteStore);

    putObject(os.get(), name, bigdata);

    const auto object = os->get(name);

    EXPECT_EQ(object.data, bigdata);
}

TEST(NatsMqObjectStoreTesting, get_big_object)
{
    constexpr auto bucket{ "testObjectStore" };
    constexpr auto name{ "objectName" };

    constexpr auto objectChunks{ 3 };
    constexpr auto chunkSize{ 128ull * 1024ull }; // 128 kB

    std::vector<uint8_t> bigdata(chunkSize * objectChunks);
    std::iota(bigdata.begin(), bigdata.end(), 0);

    const auto js = createJetStream();

    const auto config = createOSConfig(bucket);

    ObjectStorePtr os(js->getOrCreateObjectStore(config), &deleteStore);

    putObject(os.get(), name, bigdata);

    const auto object = os->get(name);

    EXPECT_EQ(object.data, bigdata);
}

TEST(NatsMqObjectStoreTesting, remove)
{
    constexpr auto bucket{ "testObjectStore" };
    constexpr auto name{ "objectName" };

    std::vector<uint8_t> bigdata;

    const auto js = createJetStream();

    const auto config = createOSConfig(bucket);

    ObjectStorePtr os(js->getOrCreateObjectStore(config), &deleteStore);

    putObject(os.get(), name, { 0x4, 0x6, 0x7, 0x4, 0x6, 0x7 });

    os->remove(name);

    const auto info = os->info(name);
    EXPECT_TRUE(info.deleted);
    const auto obj = os->get(name);
    EXPECT_TRUE(obj.meta.deleted);
}

TEST(NatsMqObjectStoreTesting, watch)
{
    constexpr auto bucket{ "testObjectStore" };
    constexpr auto name{ "objectName" };

    const auto js = createJetStream();

    const auto config = createOSConfig(bucket);

    ObjectStorePtr os(js->getOrCreateObjectStore(config), &deleteStore);

    putObject(os.get(), name, { 0x4, 0x6, 0x7, 0x4, 0x6, 0x7 });

    os->watch(name);
}

//

TEST(NatsMqObjectWatcherTesting, watcher_updated)
{
    constexpr auto bucket{ "testObjectStore" };
    constexpr auto name{ "objectName" };

    const auto js = createJetStream();

    const auto config = createOSConfig(bucket);

    ObjectStorePtr os(js->getOrCreateObjectStore(config), &deleteStore);

    putObject(os.get(), name, { 0x4, 0x6, 0x7, 0x4, 0x6, 0x7 });

    std::unique_ptr<Js::ObjectWatcher> watcher(os->watch(name));

    std::mutex              m;
    std::condition_variable cv;

    auto callCounter{ 0 };

    watcher->updated([&cv, &name, &callCounter](Js::ObjectInfo info) {
        cv.notify_one();
        ++callCounter;
        EXPECT_EQ(info.name, name);
    });

    putObject(os.get(), name, { 0x1, 0x2, 0x3, 0x4, 0x6, 0x7 });

    if (!callCounter)
    {
        std::unique_lock<std::mutex> lc(m);

        const auto status = cv.wait_for(lc, std::chrono::milliseconds(3000));
        if (status == std::cv_status::timeout && !callCounter)
            GTEST_FAIL() << "Subscription timeout";
    }
}

TEST(NatsMqObjectWatcherTesting, watcher_removed)
{
    constexpr auto bucket{ "testObjectStore" };
    constexpr auto name{ "objectName" };

    const auto js = createJetStream();

    const auto config = createOSConfig(bucket);

    ObjectStorePtr os(js->getOrCreateObjectStore(config), &deleteStore);

    putObject(os.get(), name, { 0x4, 0x6, 0x7, 0x4, 0x6, 0x7 });

    std::unique_ptr<Js::ObjectWatcher> watcher(os->watch(name));

    std::mutex              m;
    std::condition_variable cv;

    auto callCounter{ 0 };

    watcher->removed([&cv, &name, &callCounter](Js::ObjectInfo info) {
        cv.notify_one();
        ++callCounter;
        EXPECT_EQ(info.name, name);
    });

    os->remove(name);

    if (!callCounter)
    {
        std::unique_lock<std::mutex> lc(m);

        const auto status = cv.wait_for(lc, std::chrono::milliseconds(3000));
        if (status == std::cv_status::timeout && !callCounter)
            GTEST_FAIL() << "Subscription timeout";
    }
}
