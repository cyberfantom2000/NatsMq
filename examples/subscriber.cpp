#include <NatsMq>
#include <iostream>

namespace
{
    void waitSubscriptionData(NatsMq::Subscription& sub, int timeoutMs = 3000)
    {
        std::mutex              m;
        std::condition_variable cv;

        sub.registerListener([&cv](auto&& msg) { std::cout << msg.data.constData(); cv.notify_one(); });

        std::unique_lock<std::mutex> lc(m);

        const auto status = cv.wait_for(lc, std::chrono::milliseconds(timeoutMs));
        if (status == std::cv_status::timeout)
            std::cout << "Subscription timeout";
    }

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
    constexpr auto subject = "example_subject";

    std::unique_ptr<Client> client(Client::configureAndCreate());

    try
    {
        client->connect({ "nats://localhost:4222" });

        auto subscription = client->subscribe(subject);
        printStatics(subscription.statistics());

        waitSubscriptionData(subscription);
        printStatics(subscription.statistics());
    }
    catch (const NatsMq::Exception& exc)
    {
        std::cout << exc.what();
    }

    return 0;
}
