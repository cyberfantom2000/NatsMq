#include "Options.h"

#include <opts.h>

#include "Exceptions.h"

namespace
{

    natsOptions* getDefaultOptions()
    {
        natsOptions* opts;
        natsOptions_Create(&opts);
        NatsMq::exceptionIfError(natsOptions_SetNoRandomize(opts, false));
        NatsMq::exceptionIfError(natsOptions_SetTimeout(opts, NATS_OPTS_DEFAULT_TIMEOUT));
        NatsMq::exceptionIfError(natsOptions_SetVerbose(opts, false));
        NatsMq::exceptionIfError(natsOptions_SetPedantic(opts, false));
        NatsMq::exceptionIfError(natsOptions_SetPingInterval(opts, NATS_OPTS_DEFAULT_PING_INTERVAL));
        NatsMq::exceptionIfError(natsOptions_SetMaxPingsOut(opts, NATS_OPTS_DEFAULT_MAX_PING_OUT));
        NatsMq::exceptionIfError(natsOptions_SetAllowReconnect(opts, true));
        NatsMq::exceptionIfError(natsOptions_SetMaxReconnect(opts, NATS_OPTS_DEFAULT_MAX_RECONNECT));
        NatsMq::exceptionIfError(natsOptions_SetReconnectWait(opts, NATS_OPTS_DEFAULT_RECONNECT_WAIT));
        NatsMq::exceptionIfError(natsOptions_SetReconnectBufSize(opts, NATS_OPTS_DEFAULT_RECONNECT_BUF_SIZE));
        NatsMq::exceptionIfError(natsOptions_SetMaxPendingMsgs(opts, NATS_OPTS_DEFAULT_MAX_PENDING_MSGS));
        NatsMq::exceptionIfError(natsOptions_SetNoEcho(opts, false));
        NatsMq::exceptionIfError(natsOptions_SetRetryOnFailedConnect(opts, true, nullptr, nullptr));
        NatsMq::exceptionIfError(natsOptions_SetSendAsap(opts, false));
        NatsMq::exceptionIfError(natsOptions_UseGlobalMessageDelivery(opts, true));
        NatsMq::exceptionIfError(natsOptions_SetFailRequestsOnDisconnect(opts, false));
        return opts;
    }

}

NatsMq::Options::Options()
    : _natsOpts(getDefaultOptions(), &natsOptions_Destroy)
{
}

void NatsMq::Options::set(Option opt, const OptionValue& val)
{
    const auto rawOptions = _natsOpts.get();
    int        status;

    switch (opt)
    {
        case Option::Randomize:
        {
            status = natsOptions_SetNoRandomize(rawOptions, !std::get<bool>(val));
            break;
        }
        /*case Option::Secure:
        {
            status = natsOptions_SetNoRandomize(rawOptions, std::get<bool>(val));
            break;
        }*/
        case Option::Verbose:
        {
            status = natsOptions_SetVerbose(rawOptions, std::get<bool>(val));
            break;
        }
        case Option::Pedanic:
        {
            status = natsOptions_SetPedantic(rawOptions, std::get<bool>(val));
            break;
        }
        case Option::AllowRecconect:
        {
            status = natsOptions_SetRetryOnFailedConnect(rawOptions, std::get<bool>(val), nullptr, nullptr);
            break;
        }
        case Option::RetryOnFailedConnect:
        {
            status = natsOptions_SetAllowReconnect(rawOptions, std::get<bool>(val));
            break;
        }
        case Option::MaxReconnect:
        {
            status = natsOptions_SetMaxReconnect(rawOptions, std::get<int>(val));
            break;
        }
        case Option::Echo:
        {
            status = natsOptions_SetNoEcho(rawOptions, !std::get<bool>(val));
            break;
        }
        case Option::SendAsap:
        {
            status = natsOptions_SetSendAsap(rawOptions, std::get<bool>(val));
            break;
        }
        case Option::DisableNoResponders:
        {
            status = natsOptions_DisableNoResponders(rawOptions, std::get<bool>(val));
            break;
        }
        case Option::UseGlobalMsgDelivery:
        {
            status = natsOptions_UseGlobalMessageDelivery(rawOptions, std::get<bool>(val));
            break;
        }
        case Option::FailRequestOnDisconnect:
        {
            status = natsOptions_SetFailRequestsOnDisconnect(rawOptions, std::get<bool>(val));
            break;
        }
        case Option::MaxPingsOut:
        {
            status = natsOptions_SetMaxPingsOut(rawOptions, std::get<int>(val));
            break;
        }
        case Option::IOBufferSize:
        {
            status = natsOptions_SetIOBufSize(rawOptions, std::get<int>(val));
            break;
        }
        case Option::ReconnectBufferSize:
        {
            status = natsOptions_SetReconnectBufSize(rawOptions, std::get<int>(val));
            break;
        }
        case Option::MaxPendingMessages:
        {
            status = natsOptions_SetMaxPendingMsgs(rawOptions, std::get<int>(val));
            break;
        }
        case Option::Timeout:
        {
            status = natsOptions_SetTimeout(rawOptions, std::get<int64_t>(val));
            break;
        }
        case Option::PingInterval:
        {
            status = natsOptions_SetPingInterval(rawOptions, std::get<int64_t>(val));
            break;
        }
        case Option::ReconnectWait:
        {
            status = natsOptions_SetReconnectWait(rawOptions, std::get<int64_t>(val));
            break;
        }
        case Option::Name:
        {
            const auto str = std::get<std::string>(val);
            status         = natsOptions_SetName(rawOptions, str.c_str());
            break;
        }
        case Option::Token:
        {
            const auto str = std::get<std::string>(val);
            status         = natsOptions_SetToken(rawOptions, str.c_str());
            break;
        }
        case Option::UserCreds:
        {
            const auto creds = std::get<UserCredentials>(val);
            status           = natsOptions_SetUserInfo(rawOptions, creds.user.c_str(), creds.password.c_str());
            break;
        }
    }

    exceptionIfError(status);
}

natsOptions* NatsMq::Options::rawOptions() const
{
    return _natsOpts.get();
}
