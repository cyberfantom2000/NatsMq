#include <Client.h>
#include <Exceptions.h>
#include <JetStream.h>
#include <Message.h>
#include <Stream.h>
#include <gtest/gtest.h>

#include "helpers.h"

using namespace Tests;
using namespace NatsMq;

TEST(NatsMqStreamTesting, create_delete)
{
    constexpr auto streamName{ "testStream" };
    constexpr auto subject{ "testSubject" };

    const auto js     = createJetStream();
    const auto config = createConfigWithMemoryStorage(streamName, { subject });

    StreamPtr stream(js->getOrCreateStream(config), &streamDeleter);
    EXPECT_TRUE(stream->exists());
}

TEST(NatsMqStreamTesting, get_info)
{
    constexpr auto streamName{ "testStream" };
    constexpr auto subject{ "testSubject" };

    const auto js     = createJetStream();
    const auto config = createConfigWithMemoryStorage(streamName, { subject });

    StreamPtr stream(js->getOrCreateStream(config), &streamDeleter);

    const auto info = stream->info();

    // this checked not all fields
    EXPECT_EQ(info.config.name, config.name);
    EXPECT_EQ(info.config.description, config.description);
    EXPECT_EQ(info.config.maxAge, config.maxAge);
    EXPECT_EQ(info.config.maxBytes, config.maxBytes);
    EXPECT_EQ(info.config.noAck, config.noAck);
    EXPECT_EQ(info.config.discard, config.discard);
    EXPECT_EQ(info.config.storage, config.storage);
    EXPECT_EQ(info.config.subjects, config.subjects);
}

TEST(NatsMqStreamTesting, update)
{
    constexpr auto streamName{ "testStream" };
    constexpr auto subject{ "testSubject" };

    const auto js     = createJetStream();
    const auto config = createConfigWithMemoryStorage(streamName, { subject });

    StreamPtr stream(js->getOrCreateStream(config), &streamDeleter);

    Js::StreamConfig newConfig = config;
    newConfig.subjects.emplace_back("newSubject");
    newConfig.allowDirect = true;

    stream->update(newConfig);

    const auto info = stream->info();

    EXPECT_EQ(info.config.subjects, newConfig.subjects);
    EXPECT_EQ(info.config.allowDirect, newConfig.allowDirect);
}

TEST(NatsMqStreamTesting, purge)
{
    constexpr auto streamName{ "testStream" };
    constexpr auto subject{ "testSubject" };

    const auto js     = createJetStream();
    const auto config = createConfigWithMemoryStorage(streamName, { subject });

    StreamPtr stream(js->getOrCreateStream(config), &streamDeleter);

    stream->purge({});
}
