#pragma once
#include <Client.h>
#include <JetStream.h>
#include <Stream.h>

#include <memory>

#include "preferences.h"

namespace Tests
{

    inline std::unique_ptr<NatsMq::JetStream> createJetStream()
    {
        const auto client = std::unique_ptr<NatsMq::Client>(NatsMq::Client::create());
        client->connect({ natsUrl });
        return std::unique_ptr<NatsMq::JetStream>(client->jetstream());
    }

    inline NatsMq::Js::StreamConfig createConfigWithMemoryStorage(const std::string& name, const std::vector<std::string>& subjects)
    {
        NatsMq::Js::StreamConfig config;
        config.name        = name;
        config.description = name + " description";
        config.storage     = NatsMq::Js::StorageType::Memory;
        config.subjects    = subjects;
        return config;
    }

    inline void streamDeleter(NatsMq::Js::Stream* stream)
    {
        stream->remove();
        delete stream;
    }

    using StreamPtr = std::unique_ptr<NatsMq::Js::Stream, decltype(&streamDeleter)>;
}
