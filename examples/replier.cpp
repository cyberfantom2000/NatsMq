
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
        client->setOption(Option::SendAsap, true);
        client->connect({ "nats://localhost:4222" });

        auto sub = client->subscribe(subject);

        std::condition_variable cv;
        std::mutex              m;
        auto                    replyCb = [&cv, &client](IncomingMessage msg) {
            /* handle incoming msg */
            std::cout << msg.data.constData();

            /* make reply */
            msg.data = "reply_data";
            try
            {
                client->publish(msg);
            }
            catch (const Exception& exc)
            {
                std::cout << exc.what();
            }
            cv.notify_one();
        };

        sub.registerListener(replyCb);

        // wait incoming request
        std::unique_lock<std::mutex> lc(m);
    }
    catch (const NatsMq::Exception& exc)
    {
        std::cout << exc.what();
    }

    return 0;
}
