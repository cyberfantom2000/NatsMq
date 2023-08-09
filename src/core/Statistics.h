#pragma once
#include <nats.h>

#include <memory>

#include "Entities.h"

namespace NatsMq
{
    class Statistics
    {
        using NatsStatsPtr = std::unique_ptr<natsStatistics, decltype(&natsStatistics_Destroy)>;

    public:
        static IOStatistic get(natsConnection* connection);
    };
}
