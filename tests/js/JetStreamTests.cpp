#include <Client.h>
#include <Exceptions.h>
#include <JetStream.h>
#include <Message.h>
#include <Stream.h>
#include <gtest/gtest.h>

#include "helpers.h"
#include "utilitys.h"

using namespace Tests;
using namespace NatsMq;

TEST(NatsMqJetStreamTesting, stream_names)
{
    constexpr auto streamName1{ "testStream1" }, streamName2{ "testStream2" };

    const auto js = createJetStream();

    const auto config1 = createConfigWithMemoryStorage(streamName1, { "testSubject1" });
    StreamPtr  stream1(js->getOrCreateStream(config1), &streamDeleter);

    const auto config2 = createConfigWithMemoryStorage(streamName2, { "testSubject2" });
    StreamPtr  stream2(js->getOrCreateStream(config2), &streamDeleter);

    const auto names = js->streamNames();

    for (auto&& name : { streamName1, streamName2 })
    {
        const auto it = std::find(names.begin(), names.end(), std::string(name));
        EXPECT_TRUE(it != names.end());
    }
}

TEST(NatsMqJetStreamTesting, stream_infos)
{
    constexpr auto streamName1{ "testStream1" }, streamName2{ "testStream2" };

    const auto js = createJetStream();

    const auto config1 = createConfigWithMemoryStorage(streamName1, { "testSubject1" });
    StreamPtr  stream1(js->getOrCreateStream(config1), &streamDeleter);

    const auto config2 = createConfigWithMemoryStorage(streamName2, { "testSubject2" });
    StreamPtr  stream2(js->getOrCreateStream(config2), &streamDeleter);

    const auto names = js->streamsInfo();

    for (auto&& name : { streamName1, streamName2 })
    {
        const auto it = std::find_if(names.begin(), names.end(), [name = std::string(name)](const Js::StreamInfo& info) { return info.config.name == name; });
        EXPECT_TRUE(it != names.end());
    }
}

TEST(NatsMqJetStreamTesting, publish)
{
    constexpr auto streamName{ "testStream" };
    constexpr auto subject{ "testSubject" };

    const auto js     = createJetStream();
    const auto config = createConfigWithMemoryStorage(streamName, { subject });

    StreamPtr stream(js->getOrCreateStream(config), &streamDeleter);

    js->publish(msgFromString(subject, "test data message1"));
    js->publish(msgFromString(subject, "test data message2"));

    auto info = stream->info();
    EXPECT_EQ(info.state.messages, 2);
}

TEST(NatsMqJetStreamTesting, async_publish)
{
    constexpr auto streamName{ "testStream" };
    constexpr auto subject{ "testSubject" };

    const auto js     = createJetStream();
    const auto config = createConfigWithMemoryStorage(streamName, { subject });

    StreamPtr stream(js->getOrCreateStream(config), &streamDeleter);

    js->apublish(msgFromString(subject, "test data message1"));

    js->waitAsyncPublishComplete();

    auto info = stream->info();
    EXPECT_EQ(info.state.messages, 1);
}

TEST(NatsMqJetStreamTesting, async_error_handler)
{
    constexpr auto streamName{ "testStream" };
    constexpr auto subject{ "testSubject" };

    const auto js     = createJetStream();
    const auto config = createConfigWithMemoryStorage(streamName, { subject });

    int callCounter{ 0 };

    js->registerAsyncPublishErrorHandler([&callCounter](NatsMq::Message /*msg*/, NatsMq::Status /*status*/, NatsMq::Js::Status /*jsStatus*/) {
        ++callCounter;
    });

    js->apublish(msgFromString(subject, "test data message1"));

    js->waitAsyncPublishComplete();

    EXPECT_EQ(callCounter, 1);
}
