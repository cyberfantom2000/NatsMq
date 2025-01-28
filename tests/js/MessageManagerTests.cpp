#include <Client.h>
#include <Exceptions.h>
#include <JetStream.h>
#include <Message.h>
#include <MessageManager.h>
#include <Stream.h>
#include <gtest/gtest.h>

#include "helpers.h"
#include "utilitys.h"

using namespace Tests;
using namespace NatsMq;

TEST(NatsMqMessageManagerTesting, get_last_message)
{

    constexpr auto streamName{ "testStream33" };
    constexpr auto subject{ "subject123" };
    constexpr auto expectMsg{ "testSubscribeData333" };

    const auto js = createJetStream();

    auto config   = createConfigWithMemoryStorage(streamName, { subject });
    config.maxAge = 30;

    StreamPtr stream(js->getOrCreateStream(config), &streamDeleter);

    js->publish(msgFromString(subject, expectMsg));

    std::unique_ptr<Js::MessageManager> msgr(js->messageManager());

    const auto msg = msgr->getLastMessage(streamName, subject);

    const auto reply = std::string(reinterpret_cast<const char*>(msg.data.data()), msg.data.size());

    EXPECT_EQ(reply, std::string(expectMsg));
}

TEST(NatsMqMessageManagerTesting, get_message)
{

    constexpr auto streamName{ "testStream3113" };
    constexpr auto subject{ "subject3333" };
    constexpr auto expectMsg{ "test233333" };

    const auto js = createJetStream();

    auto config   = createConfigWithMemoryStorage(streamName, { subject });
    config.maxAge = 30;

    StreamPtr stream(js->getOrCreateStream(config), &streamDeleter);

    const auto ack = js->publish(msgFromString(subject, expectMsg));

    std::unique_ptr<Js::MessageManager> msgr(js->messageManager());

    const auto msg = msgr->getMessage(streamName, ack.sequence);

    const auto reply = std::string(reinterpret_cast<const char*>(msg.data.data()), msg.data.size());

    EXPECT_EQ(reply, std::string(expectMsg));
}

TEST(NatsMqMessageManagerTesting, delete_message)
{
    constexpr auto streamName{ "testStream3113" };
    constexpr auto subject{ "subject3333" };
    constexpr auto expectMsg{ "test233333" };

    const auto js = createJetStream();

    auto config   = createConfigWithMemoryStorage(streamName, { subject });
    config.maxAge = 30;

    StreamPtr stream(js->getOrCreateStream(config), &streamDeleter);

    const auto ack = js->publish(msgFromString(subject, expectMsg));

    std::unique_ptr<Js::MessageManager> msgr(js->messageManager());

    msgr->deleteMessage(streamName, ack.sequence);

    EXPECT_THROW({ msgr->getMessage(streamName, ack.sequence); }, NatsMq::JsException);
}

TEST(NatsMqMessageManagerTesting, erase_message)
{
    constexpr auto streamName{ "testStream3113" };
    constexpr auto subject{ "subject3333" };
    constexpr auto expectMsg{ "test233333" };

    const auto js = createJetStream();

    auto config   = createConfigWithMemoryStorage(streamName, { subject });
    config.maxAge = 30;

    StreamPtr stream(js->getOrCreateStream(config), &streamDeleter);

    const auto ack = js->publish(msgFromString(subject, expectMsg));

    std::unique_ptr<Js::MessageManager> msgr(js->messageManager());

    msgr->eraseMessage(streamName, ack.sequence);

    EXPECT_THROW({ msgr->getMessage(streamName, ack.sequence); }, NatsMq::JsException);
}
