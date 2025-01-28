
#include <NatsMq>
#include <iostream>

using namespace NatsMq;

namespace
{
    void deleteStore(Js::KeyValueStore* store)
    {
        if (store)
            store->deleteStore();
        delete store;
    }

    using StorePtr = std::unique_ptr<Js::KeyValueStore, decltype(&deleteStore)>;
}

int main()
{
    constexpr auto elementKey{ "new_element" };
    constexpr auto elementValue{ "new value" };

    std::unique_ptr<Client> client(Client::create());

    try
    {
        client->connect({ "nats://172.20.73.29:4222" });

        std::unique_ptr<JetStream> js(client->jetstream());

        Js::KeyValue::Config config;
        config.bucket = "example_store";

        StorePtr store(js->getOrCreateKeyValueStore(config), &deleteStore);

        store->put(elementKey, elementValue);

        const auto  value = store->get(elementKey);
        std::string valStr(reinterpret_cast<const char*>(value.value.data()), value.value.size());
        std::cout << "Value from store: " << valStr << std::endl;

        store->purge(elementKey);
    }
    catch (const NatsMq::Exception& exc)
    {
        std::cout << exc.what();
    }
    return 0;
}
