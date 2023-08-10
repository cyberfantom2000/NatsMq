
#include <NatsMq>
#include <iostream>

using namespace NatsMq;

namespace
{
    // JsStream not remove the stream on the server
    auto streamDeleter = [](JsStream* stream) {stream->remove(); delete stream; };
    using JsStreamPtr  = std::unique_ptr<JsStream, decltype(streamDeleter)>;
}

int main()
{
    constexpr auto streamName = "example_stream";
    constexpr auto subject    = "example_subject";
    constexpr auto data       = "example_data";

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

        // sync publishing, wait ack
        const auto ack = js->publish(Message(subject, data));
        std::cout << ack.stream << ack.domain;
    }
    catch (const NatsMq::Exception& exc)
    {
        std::cout << exc.what();
    }
    return 0;
}
