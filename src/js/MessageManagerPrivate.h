#pragma once

#include <nats.h>

#include <string>

namespace NatsMq
{
    class Message;

    namespace Js
    {
        class MessageManagerPrivate
        {
        public:
            MessageManagerPrivate(jsCtx* ctx);

            Message getMessage(const std::string& stream, uint64_t sequence) const;

            Message getLastMessage(const std::string& stream, const std::string& subject) const;

            void deleteMessage(const std::string& stream, uint64_t sequence) const;

            void eraseMessage(const std::string& stream, uint64_t sequence) const;

        private:
            jsCtx* _ctx;
        };

    }
}
