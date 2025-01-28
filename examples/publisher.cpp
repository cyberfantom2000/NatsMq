
#include <NatsMq>
#include <iostream>

using namespace NatsMq;

int main()
{
    constexpr auto subject = "example_subject";
    constexpr auto data    = "example_data";

    std::unique_ptr<Client> client(Client::create());

    try
    {
        client->connect({ "nats://172.20.73.29:4222" });
        client->publish(Message(subject, data));
    }
    catch (const NatsMq::Exception& exc)
    {
        std::cout << exc.what();
    }
    return 0;
}
