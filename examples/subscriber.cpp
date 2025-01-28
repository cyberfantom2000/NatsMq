#include <NatsMq>
#include <iostream>

namespace
{
    void printStatics(const NatsMq::SubscriptionStatistic& stats)
    {
        std::cout << "Pending messages: " << stats.pendingMsgs << "\n"
                  << "Pending  bytes: " << stats.pendingBytes << "\n"
                  << "Max pending messages: " << stats.maxPendingMsgs << "\n"
                  << "Max pending bytes: " << stats.maxPendingBytes << "\n"
                  << "Delivered messages: " << stats.deliveredMsgs << "\n"
                  << "Dropped messages: " << stats.droppedMsgs << std::endl;
    }
}

using namespace NatsMq;

int main()
{
    constexpr auto subject{ "example_subject" };
    constexpr auto timeoutMs{ 3000 };

    std::unique_ptr<Client> client(Client::create());

    try
    {
        client->connect({ "nats://172.20.73.29:4222" });

        std::mutex              m;
        std::condition_variable cv;

        std::unique_ptr<Subscription> subscription(client->subscribe(subject, [&cv](auto&& msg) { std::cout << std::string(msg) << std::endl; cv.notify_one(); }));
        printStatics(subscription->statistics());

        client->publish(Message(subject, "Important subscription data"));

        std::unique_lock<std::mutex> lc(m);

        // wait subscription data
        const auto status = cv.wait_for(lc, std::chrono::milliseconds(timeoutMs));
        if (status == std::cv_status::timeout)
            std::cout << "Subscription timeout";

        printStatics(subscription->statistics());
    }
    catch (const NatsMq::Exception& exc)
    {
        std::cout << exc.what();
    }

    return 0;
}
