#pragma once

#include <status.h>

#include "Entities.h"

constexpr void statusEnumValid()
{
    constexpr auto statusFirstValdid = static_cast<int>(NATS_OK) == static_cast<int>(NatsMq::Status::Ok);
    constexpr auto statusLastValid   = static_cast<int>(NATS_MISSED_HEARTBEAT) == static_cast<int>(NatsMq::Status::MissedHeartbeat);
    static_assert(statusFirstValdid && statusLastValid, "Invalid NatsMq::Status enum integrity!");
}

constexpr void connectionEnumValid()
{
    constexpr auto connectionFirstValid = static_cast<int>(NATS_CONN_STATUS_DISCONNECTED) == static_cast<int>(NatsMq::ConnectionStatus::Disconnected);
    constexpr auto connectionLastValid  = static_cast<int>(NATS_CONN_STATUS_DRAINING_PUBS) == static_cast<int>(NatsMq::ConnectionStatus::DrainingPubs);
    static_assert(connectionFirstValid && connectionLastValid, "Invalid NatsMq::ConnectionStatus enum integrity!");
}

constexpr void jsEnumValid()
{
    constexpr auto firstPartStartValid  = static_cast<int>(JSAccountResourcesExceededErr) == static_cast<int>(NatsMq::Js::Status::AccountResourcesExceededErr);
    constexpr auto firstPartEndValid    = static_cast<int>(JSStreamInvalidErr) == static_cast<int>(NatsMq::Js::Status::StreamInvalidErr);
    constexpr auto secondPartStartValid = static_cast<int>(JSConsumerWQRequiresExplicitAckErr) == static_cast<int>(NatsMq::Js::Status::ConsumerWQRequiresExplicitAckErr);
    constexpr auto secondPartEndValid   = static_cast<int>(JSStreamInfoMaxSubjectsErr) == static_cast<int>(NatsMq::Js::Status::StreamInfoMaxSubjectsErr);

    static_assert(firstPartStartValid && firstPartEndValid && secondPartStartValid && secondPartEndValid, "Invalid NatsMq::ConnectionStatus enum integrity!");
}

constexpr void staticCheckEnumIntegrity()
{
    statusEnumValid();
    connectionEnumValid();
    jsEnumValid();
}
