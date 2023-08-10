
#include <NatsMq>
#include <iostream>

using namespace NatsMq;

namespace
{
    // JsStream not remove the stream on the server
    auto streamDeleter = [](JsStream* stream) {stream->remove(); delete stream; };
    using JsStreamPtr  = std::unique_ptr<JsStream, decltype(streamDeleter)>;

    void waitSubscriptionData(NatsMq::JsSubscription& sub, int timeoutMs = 3000)
    {
        std::mutex              m;
        std::condition_variable cv;

        sub.registerListener([&cv](auto&& msg) {
            std::cout << msg.data.constData();
            msg.ack(); // you must manual ack or nack msg
            cv.notify_one();
        });

        std::unique_lock<std::mutex> lc(m);

        const auto status = cv.wait_for(lc, std::chrono::milliseconds(timeoutMs));
        if (status == std::cv_status::timeout)
            std::cout << "Subscription timeout";
    }
}

int main()
{
    constexpr auto streamName = "example_stream";
    constexpr auto subject    = "example_subject";

    std::unique_ptr<Client> client(Client::create());

    try
    {
        client->connect({ "nats://localhost:4222" });
        std::unique_ptr<JetStream> js(client->createJetStream());

        JsStreamConfig config;
        config.name     = streamName;
        config.storage  = JsStreamConfig::Storage::Memory;
        config.subjects = { subject };

        // stream cannot be used after destroying js object
        JsStreamPtr ptr(js->getOrCreateStream(config), streamDeleter);

        auto sub = js->subscribe(config.name, config.subjects.front());
        waitSubscriptionData(sub);
    }
    catch (const NatsMq::Exception& exc)
    {
        std::cout << exc.what();
    }
    return 0;
}
