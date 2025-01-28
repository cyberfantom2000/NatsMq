#include <Client.h>
#include <Exceptions.h>
#include <JetStream.h>
#include <Message.h>
#include <PullSubscription.h>
#include <Stream.h>
#include <gtest/gtest.h>

#include "helpers.h"
#include "utilitys.h"

using namespace Tests;
using namespace NatsMq;

TEST(NatsMqJsSubscriptionTesting, subscribe)
{
    constexpr auto streamName{ "testStreamSub" };
    constexpr auto subject{ "testSubjectSub" };
    constexpr auto expectMsg{ "testSubscribeData" };

    const auto js     = createJetStream();
    const auto config = createConfigWithMemoryStorage(streamName, { subject });

    StreamPtr stream(js->getOrCreateStream(config), &streamDeleter);

    std::mutex              m;
    std::condition_variable cv;

    auto cb = [&expectMsg, &cv](NatsMq::Js::IncomingMessage msg) {
        cv.notify_one();
        const auto reply = std::string(reinterpret_cast<const char*>(msg.msg.data.data()), msg.msg.data.size());
        msg.ack();
        EXPECT_EQ(expectMsg, reply);
    };

    std::unique_ptr<Js::Subscription> sub(js->subscribe(subject, streamName, std::move(cb)));

    QStringList arguments{ pyFile, "--publish_subject", subject, "--publish_data", expectMsg, "--nats_address", natsAddress };
    const auto  process = createProcess();
    process->start(python, arguments);

    std::unique_lock<std::mutex> lc(m);

    const auto status = cv.wait_for(lc, std::chrono::milliseconds(3000));

    if (status == std::cv_status::timeout)
        GTEST_FAIL() << "Subscription timeout";
}

TEST(NatsMqJsSubscriptionTesting, sync_subscribe_next)
{
    constexpr auto streamName{ "testStreamSub" };
    constexpr auto subject{ "testSubjectSub" };
    constexpr auto expectMsg{ "testSubscribeData" };

    const auto js     = createJetStream();
    const auto config = createConfigWithMemoryStorage(streamName, { subject });

    StreamPtr stream(js->getOrCreateStream(config), &streamDeleter);

    std::unique_ptr<Js::SyncSubscription> sub(js->syncSubscribe(subject, streamName));

    QStringList arguments{ pyFile, "--publish_subject", subject, "--publish_data", expectMsg, "--nats_address", natsAddress };
    const auto  process = createProcess();
    process->start(python, arguments);

    auto msg = sub->next(2000);
    msg.ack();
    const auto reply = std::string(reinterpret_cast<const char*>(msg.msg.data.data()), msg.msg.data.size());
    EXPECT_EQ(expectMsg, reply);
}

TEST(NatsMqJsSubscriptionTesting, pull_subscribe_fetch)
{
    constexpr auto streamName{ "testStreamSub1111" };
    constexpr auto subject{ "testSubjectSub11111" };
    constexpr auto expectMsg{ "testSubscribeData1111" };

    const auto js     = createJetStream();
    const auto config = createConfigWithMemoryStorage(streamName, { subject });

    StreamPtr stream(js->getOrCreateStream(config), &streamDeleter);

    std::unique_ptr<Js::PullSubscription> sub(js->pullSubscribe(subject, streamName));

    QStringList arguments{ pyFile, "--publish_subject", subject, "--publish_data", expectMsg, "--nats_address", natsAddress };

    constexpr auto msgCount{ 3 };

    for (auto i = 0; i < msgCount; ++i)
    {
        const auto process = createProcess();
        process->start(python, arguments);
    }

    auto msgs = sub->fetch(msgCount);

    EXPECT_EQ(msgs.size(), msgCount);

    for (auto&& msg : msgs)
    {
        msg.ack();
        const auto reply = std::string(reinterpret_cast<const char*>(msg.msg.data.data()), msg.msg.data.size());
        EXPECT_EQ(expectMsg, reply);
    }
}

TEST(NatsMqJsSubscriptionTesting, headers)
{
    constexpr auto streamName{ "testStreamSub11sss11" };
    constexpr auto subject{ "testsub" };

    const auto js     = createJetStream();
    const auto config = createConfigWithMemoryStorage(streamName, { subject });

    StreamPtr stream(js->getOrCreateStream(config), &streamDeleter);

    std::mutex              m;
    std::condition_variable cv;

    Message::Headers headers{ { "key1", "val1" }, { "key2", "val2" }, { "key3", "val3" }, { "key4", "val4" } };

    auto hasCall{ false };

    auto cb = [&headers, &cv, &hasCall](Js::IncomingMessage msg) {
        EXPECT_EQ(headers, msg.msg.headers);
        hasCall = true;
        cv.notify_all();
    };

    std::unique_ptr<Js::Subscription> sub(js->subscribe(subject, streamName, std::move(cb)));

    Message msg(subject, "Important subscription data");
    msg.headers = headers;
    js->publish(std::move(msg));

    std::unique_lock<std::mutex> lc(m);

    if (!hasCall)
    {
        const auto status = cv.wait_for(lc, std::chrono::milliseconds(3000));
        if (status == std::cv_status::timeout && !hasCall)
            GTEST_FAIL() << "Subscription timeout";
    }
}
