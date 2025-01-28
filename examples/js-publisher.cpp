
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
    constexpr auto streamName = "example_stream";
    constexpr auto subject    = "example_subject";
    constexpr auto data       = "example_data";

    std::unique_ptr<Client> client(Client::create());

    try
    {
        client->connect({ "nats://172.20.73.29:4222" });
        std::unique_ptr<JetStream> js(client->jetstream());

        Js::StreamConfig config;
        config.name     = streamName;
        config.storage  = Js::StorageType::Memory;
        config.subjects = { subject };

        // stream cannot be used after destroying js object
        JsStreamPtr ptr(js->getOrCreateStream(config), streamDeleter);

        // sync publishing, wait ack
        const auto ack = js->publish(Message(subject, data));
        std::cout << ack.stream << ack.domain << std::endl;
    }
    catch (const NatsMq::Exception& exc)
    {
        std::cout << exc.what();
    }
    return 0;
}
