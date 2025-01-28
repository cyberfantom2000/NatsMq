
#include <NatsMq>
#include <condition_variable>
#include <iostream>
#include <mutex>

using namespace NatsMq;

int main()
{
    constexpr auto subject = "example_subject";

    std::unique_ptr<Client> client(Client::create());

    try
    {
        ConnectionOptions options;
        options.sendAsap = true; // sending messages without delay

        client->connect({ "nats://172.20.73.29:4222" });

        auto replyCb = [&client](Message msg) {
            /* handle incoming msg */
            std::cout << std::string(msg) << std::endl;

            /* make reply */
            std::string reply = "it is request callback reply";

            msg.subject = msg.replySubject;
            msg.replySubject.clear();
            msg.data.clear();
            msg.data.insert(msg.data.end(), reply.begin(), reply.end());

            try
            {
                client->publish(std::move(msg));
            }
            catch (const Exception& exc)
            {
                std::cout << exc.what();
            }
        };

        std::unique_ptr<Subscription> sub(client->subscribe(subject, std::move(replyCb)));

        auto reply = client->request(Message(subject, "it is request callback data"));
        std::cout << std::string(reply) << std::endl;
    }
    catch (const NatsMq::Exception& exc)
    {
        std::cout << exc.what();
    }

    return 0;
}
