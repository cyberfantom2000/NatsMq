#include "Publisher.h"

#include "Exceptions.h"
#include "Utils.h"

NatsMq::Publisher::Publisher(natsConnection* connection)
    : _connection(connection)
{
}

void NatsMq::Publisher::puslish(const Message& msg)
{
    const auto natsMsg = createNatsMessage(msg);
    exceptionIfError(natsConnection_PublishMsg(_connection, natsMsg.get()));
}
