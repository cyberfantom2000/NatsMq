
#include <NatsMq>
#include <iostream>

using namespace NatsMq;

namespace
{
    auto storeDeleter = [](Js::ObjectStore* os) {if(os)os->deleteStore(); delete os; };
    using StorePtr    = std::unique_ptr<Js::ObjectStore, decltype(storeDeleter)>;

    void printInfo(const Js::ObjectInfo& info)
    {
        std::cout << "Name: " << info.name << '\n'
                  << "Size: " << info.size << '\n'
                  << "Chunks: " << info.chunks << '\n'
                  << "Bucket: " << info.bucket << std::endl;
    }
}

int main()
{
    constexpr auto streamName = "example_stream";
    constexpr auto data       = "example_data";

    std::unique_ptr<Client> client(Client::create());

    try
    {
        client->connect({ "nats://172.20.73.29:4222" });
        std::unique_ptr<JetStream> js(client->jetstream());

        Js::ObjectStoreConfig config;
        config.bucket  = streamName;
        config.storage = Js::StorageType::Memory;
        config.maxAge  = 60;

        StorePtr os(js->getOrCreateObjectStore(config), storeDeleter);

        Js::ObjectElement el;
        el.meta.name = "object_name";

        std::string d(data);
        el.data.insert(el.data.end(), d.begin(), d.end());

        // put object to storage
        os->put(el);

        // get only object info
        const auto info = os->info(el.meta.name);
        printInfo(info);

        // get object info and data
        const auto object = os->get(el.meta.name);
        printInfo(object.meta);

        // remove object. if request object or info after remove then meta.deleted == true
        os->remove(el.meta.name);
    }
    catch (const NatsMq::Exception& exc)
    {
        std::cout << exc.what();
    }
    return 0;
}
