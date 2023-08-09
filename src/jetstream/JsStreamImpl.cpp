#include "JsStreamImpl.h"

#include "Exceptions.h"
#include "JsStream.h"

namespace
{
    using StreamInfoPtr = std::unique_ptr<jsStreamInfo, decltype(&jsStreamInfo_Destroy)>;

    std::vector<const char*> createElementPointers(const std::vector<std::string>& elements)
    {
        std::vector<const char*> pointers;
        for (auto&& element : elements)
            pointers.push_back(element.c_str());
        return pointers;
    }

    jsStreamConfig createCNatsConfig(const std::string& name, std::vector<const char*>& subjects, NatsMq::JsStreamConfig::Storage storage)
    {
        jsStreamConfig config;
        jsStreamConfig_Init(&config);

        config.Name        = name.c_str();
        config.Subjects    = subjects.data();
        config.SubjectsLen = static_cast<int>(subjects.size());
        config.Storage     = static_cast<jsStorageType>(storage);

        return config;
    }
}

NatsMq::JsStream::JsStream(NatsMq::JsStreamImpl* impl)
    : _impl(impl)
{
}

NatsMq::JsStream::~JsStream() = default;

void NatsMq::JsStream::update(const JsStreamConfig& config)
{
    _impl->update(config);
}

void NatsMq::JsStream::purge() const
{
    _impl->purge();
}

void NatsMq::JsStream::remove() const
{
    _impl->remove();
}

bool NatsMq::JsStream::exists() const
{
    _impl->exists();
}

NatsMq::JsStreamImpl* NatsMq::JsStreamImpl::getOrCreate(jsCtx* context, const JsStreamConfig& config)
{
    const auto stream = JsStreamImpl::get(context, config);
    if (!stream->exists())
        stream->create();
    return stream;
}

NatsMq::JsStreamImpl* NatsMq::JsStreamImpl::get(jsCtx* context, const JsStreamConfig& config)
{
    return new JsStreamImpl(context, config);
}

NatsMq::JsStreamImpl::JsStreamImpl(jsCtx* context, const JsStreamConfig& config)
    : _context(context)
    , _config(config)
{
}

void NatsMq::JsStreamImpl::update(const JsStreamConfig& newConfig)
{
    auto subjects = createElementPointers(newConfig.subjects);
    auto config   = createCNatsConfig(newConfig.name, subjects, newConfig.storage);

    jsErrCode  jerr;
    const auto status = js_UpdateStream(nullptr, _context, &config, nullptr, &jerr);

    jsExceptionIfError(status, jerr);
    _config = newConfig;
}

void NatsMq::JsStreamImpl::purge() const
{
    jsErrCode  jerr;
    const auto status = js_PurgeStream(_context, _config.name.c_str(), nullptr, &jerr);
    jsExceptionIfError(status, jerr);
}

void NatsMq::JsStreamImpl::remove() const
{
    jsErrCode  jerr;
    const auto status = js_DeleteStream(_context, _config.name.c_str(), nullptr, &jerr);
    jsExceptionIfError(status, jerr);
}

bool NatsMq::JsStreamImpl::exists() const
{
    jsErrCode     jerr;
    jsStreamInfo* natsInfo{ nullptr };

    const auto    status = js_GetStreamInfo(&natsInfo, _context, _config.name.c_str(), nullptr, &jerr);
    StreamInfoPtr info(natsInfo, &jsStreamInfo_Destroy);

    if (status == NATS_NOT_FOUND)
        return false;

    jsExceptionIfError(status, jerr);
    return true;
}

void NatsMq::JsStreamImpl::create() const
{
    auto subjects = createElementPointers(_config.subjects);
    auto config   = createCNatsConfig(_config.name, subjects, _config.storage);

    jsErrCode  jerr;
    const auto status = js_AddStream(nullptr, _context, &config, nullptr, &jerr);

    jsExceptionIfError(status, jerr);
}
