#include <Client.h>
#include <Exceptions.h>
#include <Message.h>
#include <gtest/gtest.h>

#include "preferences.h"
#include "utilitys.h"

using namespace Tests;

TEST(NatsMqClientTesting, connection_success)
{
    std::unique_ptr<SignalEmitCounter> catcher;

    const auto client = std::unique_ptr<NatsMq::Client>(NatsMq::Client::create());

    catcher = std::make_unique<SignalEmitCounter>(client.get(), 3, 0);

    client->connect({ natsUrl });

    EXPECT_TRUE(client->ping(1000));
}

TEST(NatsMqClientTesting, connection_error)
{
    std::unique_ptr<SignalEmitCounter> catcher;

    const auto client = std::unique_ptr<NatsMq::Client>(NatsMq::Client::create());

    catcher = std::make_unique<SignalEmitCounter>(client.get(), 1);

    NatsMq::ConnectionOptions options;
    options.maxReconnect  = 1;
    options.reconnectWait = 10;

    EXPECT_THROW({ client->connect({ "nats://localhost:1111" }, options); }, NatsMq::Exception);
}

TEST(NatsMqClientTesting, disconnect_success)
{
    std::unique_ptr<SignalEmitCounter> catcher;

    const auto client = std::unique_ptr<NatsMq::Client>(NatsMq::Client::create());

    catcher = std::make_unique<SignalEmitCounter>(client.get(), 3, 0);

    client->connect({ natsUrl });

    client->disconnect();

    EXPECT_FALSE(client->ping(1000));
}

TEST(NatsMqClientTesting, reconnect)
{
    std::unique_ptr<SignalEmitCounter> catcher;

    const auto client = std::unique_ptr<NatsMq::Client>(NatsMq::Client::create());

    catcher = std::make_unique<SignalEmitCounter>(client.get(), 6, 0);

    client->connect({ natsUrl });

    client->disconnect();

    client->connect({ natsUrl });

    EXPECT_TRUE(client->ping(1000));
}

TEST(NatsMqClientTesting, attempt_publish)
{
    const auto client = std::unique_ptr<NatsMq::Client>(NatsMq::Client::create());
    client->connect({ natsUrl });

    client->publish(msgFromString("test", "test one two"));
}

TEST(NatsMqClientTesting, request_error)
{
    auto cb = []() {
        try
        {
            const auto client = std::unique_ptr<NatsMq::Client>(NatsMq::Client::create());
            client->connect({ natsUrl });
            client->request(msgFromString("test123", "test222"));
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

TEST(NatsMqClientTesting, request_with_responder)
{
    constexpr auto expectMsg{ "ttt" };
    constexpr auto subject{ "test" };

    QStringList arguments{ pyFile, "--reply_subject", subject, "--reply_data", expectMsg, "--nats_address", natsAddress };

    const auto process    = createProcess();
    const auto descriptor = ProcessDescriptor{ 15000, python, arguments };

    auto cb = []() {
        const auto client = std::unique_ptr<NatsMq::Client>(NatsMq::Client::create());
        client->connect({ natsUrl });
        return client->request(msgFromString(subject, "request params"), 10000);
    };

    const auto msg = callAfterStart(process.get(), descriptor, cb);

    const auto reply = std::string(reinterpret_cast<const char*>(msg.data.data()), msg.data.size());
    EXPECT_EQ(reply, std::string(expectMsg));
}

TEST(NatsMqClientTesting, async_request_error)
{
    constexpr auto subject{ "test" };

    const auto client = std::unique_ptr<NatsMq::Client>(NatsMq::Client::create());
    client->connect({ natsUrl });

    auto futureMsg = client->arequest(msgFromString(subject, "test123"), 200);
    EXPECT_THROW({ auto msg = futureMsg.get(); }, NatsMq::Exception);
}

TEST(NatsMqClientTesting, async_request_with_responder)
{
    constexpr auto expectMsg{ "test1" };
    constexpr auto subject{ "test" };

    QStringList arguments{ pyFile, "--reply_subject", subject, "--reply_data", expectMsg, "--nats_address", natsAddress };

    const auto process    = createProcess();
    const auto descriptor = ProcessDescriptor{ 15000, python, arguments };

    const auto client = std::unique_ptr<NatsMq::Client>(NatsMq::Client::create());

    auto cb = [&]() {
        client->connect({ natsUrl });
        return client->arequest(msgFromString(subject, "test123"), 10000);
    };

    auto futureMsg = callAfterStart(process.get(), descriptor, cb);

    const auto msg   = futureMsg.get();
    const auto reply = std::string(reinterpret_cast<const char*>(msg.data.data()), msg.data.size());

    EXPECT_EQ(reply, std::string(expectMsg));
}

TEST(NatsMqClientTesting, reply)
{
    constexpr auto expectMsgCb{ "test1" };
    constexpr auto expectMsgReply{ "test2231" };
    constexpr auto subject{ "test" };

    const auto client = std::unique_ptr<NatsMq::Client>(NatsMq::Client::create());
    client->connect({ natsUrl });

    auto replyCb = [&client, expectMsgCb](NatsMq::Message msg) {
        EXPECT_EQ(expectMsgCb, std::string(msg));

        msg.subject = msg.replySubject;
        msg.replySubject.clear();
        msg.data.clear();
        msg.data.insert(msg.data.end(), expectMsgReply, expectMsgReply + sizeof(expectMsgReply));
        client->publish(std::move(msg));
    };

    std::unique_ptr<NatsMq::Subscription> sub(client->subscribe(subject, std::move(replyCb)));

    const auto reply = client->request(NatsMq::Message(subject, expectMsgCb));
    EXPECT_EQ(expectMsgReply, std::string(reply));
}
