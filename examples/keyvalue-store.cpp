
#include <NatsMq>
#include <iostream>

using namespace NatsMq;

int main()
{
    std::unique_ptr<Client> client(Client::configureAndCreate());

    try
    {
        client->connect({ "nats://localhost:4222" });

        std::unique_ptr<JetStream> js(client->createJetStream());

        KeyValueConfig config;
        config.name = "example_store";

        const auto store = js->getOrCreateStore(config);

        store->putElement("new_element", "new_value");

        const auto value = store->getElement("new_element");
        std::cout << "Value from store: " << value.constData() << std::endl;

        store->purgeElement("new_element");

        js->deleteStore(store->storeName());
    }
    catch (const NatsMq::Exception& exc)
    {
        std::cout << exc.what();
    }
    return 0;
}
