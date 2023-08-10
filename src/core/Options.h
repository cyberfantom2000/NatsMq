#pragma once

#include <nats.h>

#include <memory>

#include "Entities.h"

namespace NatsMq
{
    class Options
    {
    public:
        using NatsOptionsPtr = std::unique_ptr<natsOptions, decltype(&natsOptions_Destroy)>;

        Options();

        void set(Option opt, const OptionValue& val);

        natsOptions* rawOptions() const;

    private:
        NatsOptionsPtr _natsOpts;
    };
}
