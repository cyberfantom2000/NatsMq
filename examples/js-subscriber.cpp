
#include <NatsMq>
#include <iostream>

using namespace NatsMq;

namespace
{
    // JsStream not remove the stream on the server
    auto streamDeleter = [](Js::Stream* stream) {stream->remove(); delete stream; };
    using JsStreamPtr  = std::unique_ptr<Js::Stream, decltype(streamDeleter)>;
}

int main()
{
    constexpr auto streamName{ "example_stream" };
    constexpr auto subject{ "example_subject" };
    constexpr auto timeoutMs{ 3000 };

    std::unique_ptr<Client> client(Client::create());

    try
    {
        client->connect({ "nats://172.20.73.29:4222" });
        std::unique_ptr<JetStream> js(client->jetstream());

        Js::StreamConfig config;
        config.name     = streamName;
        config.storage  = Js::StorageType::Memory;
        config.subjects = { subject };

        std::mutex              m;
        std::condition_variable cv;

        // stream cannot be used after destroying js object
        JsStreamPtr ptr(js->getOrCreateStream(config), streamDeleter);

        std::unique_ptr<Js::Subscription> sub(js->subscribe(subject, streamName, //
                                                            [&cv](auto&& msg) { std::cout << std::string(msg) << std::endl; cv.notify_one(); }));

        js->publish(Message(subject, "Important subscription data"));

        std::unique_lock<std::mutex> lc(m);

        // wait subscription data
        const auto status = cv.wait_for(lc, std::chrono::milliseconds(timeoutMs));
        if (status == std::cv_status::timeout)
            std::cout << "Subscription timeout";
    }
    catch (const NatsMq::Exception& exc)
    {
        std::cout << exc.what();
    }
    return 0;
}
