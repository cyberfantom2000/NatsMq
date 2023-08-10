
#include <NatsMq>
#include <iostream>

#define SYNC

using namespace NatsMq;

int main()
{
    constexpr auto subject{ "example_subject" };
    constexpr auto data{ "example_data" };
    constexpr auto timeoutMs{ 3000 };

    std::unique_ptr<Client> client(Client::create());

    try
    {
        client->connect({ "nats://localhost:4222" });
#ifdef SYNC
        const auto msg = client->request(Message(subject, data), timeoutMs);
#else
        auto futureMsg = client->asyncReuest(Message(subject, data), timeoutMs);

        const auto msg = futureMsg.get();
#endif
        std::cout << msg.data.constData();
    }
    catch (const NatsMq::Exception& exc)
    {
        std::cout << exc.what();
    }
    return 0;
}
