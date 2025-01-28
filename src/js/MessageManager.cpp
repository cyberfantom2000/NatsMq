#include "MessageManager.h"

#include "js/MessageManagerPrivate.h"

using namespace NatsMq;

Js::MessageManager::MessageManager(MessageManagerPrivate* impl)
    : _impl(impl)
{
}

Js::MessageManager::~MessageManager() = default;

Js::MessageManager::MessageManager(MessageManager&&) = default;

Js::MessageManager& Js::MessageManager::operator=(MessageManager&&) = default;

Message Js::MessageManager::getLastMessage(const std::string& stream, const std::string& subject) const
{
    return _impl->getLastMessage(stream, subject);
}

Message Js::MessageManager::getMessage(const std::string& stream, uint64_t sequence) const
{
    return _impl->getMessage(stream, sequence);
}

void Js::MessageManager::deleteMessage(const std::string& stream, uint64_t sequence) const
{
    _impl->deleteMessage(stream, sequence);
}

void Js::MessageManager::eraseMessage(const std::string& stream, uint64_t sequence) const
{
    _impl->eraseMessage(stream, sequence);
}
