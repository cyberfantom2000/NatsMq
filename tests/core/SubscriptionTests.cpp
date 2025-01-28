#include <Client.h>
#include <Exceptions.h>
#include <Message.h>
#include <gtest/gtest.h>

#include <QDebug>

#include "preferences.h"
#include "utilitys.h"

using namespace Tests;

TEST(NatsMqSubscriptionTesting, subscribe)
{
    constexpr auto expectMsg{ "test_subscribe" };
    constexpr auto subject{ "testsub" };

    const auto client = std::unique_ptr<NatsMq::Client>(NatsMq::Client::create());
    client->connect({ natsUrl });

    std::mutex              m;
    std::condition_variable cv;

    auto cb = [&expectMsg, &cv](auto&& msg) {
        cv.notify_one();
        const auto reply = std::string(reinterpret_cast<const char*>(msg.data.data()), msg.data.size());
        EXPECT_EQ(expectMsg, reply);
    };

    std::unique_ptr<NatsMq::Subscription> sub(client->subscribe(subject, std::move(cb)));

    QStringList arguments{ pyFile, "--publish_subject", subject, "--publish_data", expectMsg, "--nats_address", natsAddress };
    const auto  process = createProcess();
    process->start(python, arguments);

    std::unique_lock<std::mutex> lc(m);

    const auto status = cv.wait_for(lc, std::chrono::milliseconds(3000));
    if (status == std::cv_status::timeout)
        GTEST_FAIL() << "Subscription timeout";
}

TEST(NatsMqSubscriptionTesting, sync_subscribe)
{
    constexpr auto expectMsg{ "test_sync_subscribe" };
    constexpr auto subject{ "testsub_sync" };

    QStringList arguments{ pyFile, "--publish_subject", subject, "--publish_data", expectMsg, "--publish_timeout", "3", "--nats_address", natsAddress };
    const auto  process = createProcess();
    process->start(python, arguments);

    const auto client = std::unique_ptr<NatsMq::Client>(NatsMq::Client::create());
    client->connect({ natsUrl });
    std::unique_ptr<NatsMq::SyncSubscription> sub(client->syncSubscribe(subject));

    const auto msg   = sub->next(10000);
    const auto reply = std::string(reinterpret_cast<const char*>(msg.data.data()), msg.data.size());

    EXPECT_EQ(reply, std::string(expectMsg));
}

TEST(NatsMqSubscriptionTesting, headers)
{
    constexpr auto subject{ "testsub" };

    const auto client = std::unique_ptr<NatsMq::Client>(NatsMq::Client::create());
    client->connect({ natsUrl });

    std::mutex              m;
    std::condition_variable cv;

    NatsMq::Message::Headers headers{ { "key1", "val1" }, { "key2", "val2" }, { "key3", "val3" }, { "key4", "val4" } };

    auto cb = [&headers, &cv](NatsMq::Message msg) {
        EXPECT_EQ(headers, msg.headers);
        cv.notify_all();
    };

    std::unique_ptr<NatsMq::Subscription> sub(client->subscribe(subject, std::move(cb)));

    NatsMq::Message msg(subject, "Important subscription data");
    msg.headers = headers;
    client->publish(std::move(msg));

    std::unique_lock<std::mutex> lc(m);

    const auto status = cv.wait_for(lc, std::chrono::milliseconds(3000));
    if (status == std::cv_status::timeout)
        GTEST_FAIL() << "Subscription timeout";
}

// TODO
// drain, messageCount, statistics
