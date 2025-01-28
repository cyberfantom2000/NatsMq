#include "Publisher.h"

#include "Exceptions.h"
#include "Message.h"
#include "private/utils.h"

using namespace NatsMq;

Publisher::Publisher(natsConnection* connection)
    : _connection(connection)
{
}

void Publisher::publish(Message msg) const
{
    const auto cnatsMsg = createCnatsMessage(msg);
    exceptionIfError(natsConnection_PublishMsg(_connection, cnatsMsg.get()));
}

void Publisher::publish(std::string subject, std::string data) const
{
    exceptionIfError(natsConnection_PublishString(_connection, subject.c_str(), data.c_str()));
}
