#include "Statistics.h"

#include "Exceptions.h"

NatsMq::IOStatistic NatsMq::Statistics::get(natsConnection* connection)
{
    natsStatistics* natsStats;
    exceptionIfError(natsStatistics_Create(&natsStats));

    NatsStatsPtr stats(natsStats, &natsStatistics_Destroy);
    exceptionIfError(natsConnection_GetStats(connection, natsStats));

    IOStatistic out;

    exceptionIfError(natsStatistics_GetCounts(natsStats, &out.inMsgs, &out.inBytes,
                                              &out.outMsgs, &out.outBytes, &out.reconnected));

    return out;
}
