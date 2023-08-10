#include <gtest/gtest.h>

#include <condition_variable>
#include <future>
#include <iostream>

#include "Client.h"
#include "Entities.h"
#include "Exceptions.h"
#include "TestUtils.h"

using namespace Tests;

namespace
{
    constexpr auto subject{ "test_subject" };
}

TEST(NatsMqCoreTesting, connection_success)
{
    std::unique_ptr<SignalEmitCounter> catcher;

    const auto client = std::unique_ptr<NatsMq::Client>(NatsMq::Client::create());

    catcher = std::make_unique<SignalEmitCounter>(client.get(), 3, 0);

    client->connect({ validNatsUrl });

    EXPECT_TRUE(client->pingServer(1000));
}

TEST(NatsMqCoreTesting, connection_error)
{
    std::unique_ptr<SignalEmitCounter> catcher;

    const auto client = std::unique_ptr<NatsMq::Client>(NatsMq::Client::create());

    catcher = std::make_unique<SignalEmitCounter>(client.get(), 1);

    client->setOption(NatsMq::Option::MaxReconnect, 1);
    client->setOption(NatsMq::Option::ReconnectWait, 10ll);

    EXPECT_THROW({ client->connect({ "nats://localhost:1111" }); }, NatsMq::Exception);
}

TEST(NatsMqCoreTesting, attempt_publish)
{
    const auto client = std::unique_ptr<NatsMq::Client>(NatsMq::Client::create());
    client->connect({ validNatsUrl });
    client->publish(NatsMq::Message{ "test", "test one two" });
}

TEST(NatsMqCoreTesting, request_error)
{
    auto cb = []() {
        try
        {
            const auto client = std::unique_ptr<NatsMq::Client>(NatsMq::Client::create());
            client->connect({ validNatsUrl });
            client->request(NatsMq::Message{ "test", "test" });
        }
        catch (const NatsMq::Exception& exc)
        {
            if (exc.status != NatsMq::Status::NoResponders)
                GTEST_FAIL() << "Invalid status";
            throw;
        }
    };

    EXPECT_THROW({ cb(); }, NatsMq::Exception);
}

TEST(NatsMqCoreTesting, request_with_responder)
{
    constexpr auto expectMsg{ "test reply" };

    QStringList arguments{ pyFile, "--reply_subject", subject, "--reply_data", expectMsg };

    const auto process    = createProcess();
    const auto descriptor = ProcessDescriptor{ 15000, python, arguments };

    auto cb = []() {
        const auto client = std::unique_ptr<NatsMq::Client>(NatsMq::Client::create());
        client->connect({ validNatsUrl });
        return client->request(NatsMq::Message{ subject, "test123" }, 10000);
    };

    const auto msg = callAfterStart(process.get(), descriptor, cb);
    auto       a   = std::string(msg.data.constData(), msg.data.size());
    EXPECT_EQ(std::string(msg.data.constData()), std::string(expectMsg));
}

TEST(NatsMqCoreTesting, async_request_error)
{
    const auto client = std::unique_ptr<NatsMq::Client>(NatsMq::Client::create());
    client->connect({ validNatsUrl });

    auto futureMsg = client->asyncReuest(NatsMq::Message{ subject, "test123" }, 200);
    EXPECT_THROW({ auto msg = futureMsg.get(); }, NatsMq::Exception);
}

TEST(NatsMqCoreTesting, async_request_with_responder)
{
    constexpr auto expectMsg{ "test1" };

    QStringList arguments{ pyFile, "--reply_subject", subject, "--reply_data", expectMsg };

    const auto process    = createProcess();
    const auto descriptor = ProcessDescriptor{ 15000, python, arguments };

    const auto client = std::unique_ptr<NatsMq::Client>(NatsMq::Client::create());

    auto cb = [&]() {
        client->connect({ validNatsUrl });
        return client->asyncReuest(NatsMq::Message{ subject, "test123" }, 10000);
    };

    auto futureMsg = callAfterStart(process.get(), descriptor, cb);

    const auto msg = futureMsg.get();

    EXPECT_EQ(std::string(msg.data.constData()), std::string(expectMsg));
}

// TODO
//TEST(NatsMqCoreTesting, reply_on_request)
//{
//}

TEST(NatsMqCoreTesting, subscribe)
{
    constexpr auto expectMsg{ "test_subscribe" };

    const auto client = std::unique_ptr<NatsMq::Client>(NatsMq::Client::create());
    client->connect({ validNatsUrl });
    auto sub = client->subscribe(subject);

    QStringList arguments{ pyFile, "--publish_subject", subject, "--publish_data", expectMsg };
    const auto  process = createProcess();
    process->start(python, arguments);

    std::condition_variable cv;

    waitSubscriptionData<NatsMq::Subscription>(sub, NatsMq::ByteArray(expectMsg));
}
