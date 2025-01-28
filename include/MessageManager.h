#pragma once

#include "Export.h"
#include "Message.h"

namespace NatsMq
{
    namespace Js
    {
        class MessageManagerPrivate;

        class NATSMQ_EXPORT MessageManager
        {
        public:
            MessageManager(MessageManagerPrivate* impl);

            ~MessageManager();

            MessageManager(MessageManager&&);

            MessageManager& operator=(MessageManager&&);

            //! Retrieves a raw stream message stored in JetStream by sequence number.
            Message getMessage(const std::string& stream, uint64_t sequence) const;

            //! Retrieves the last JetStream message from the stream for a given subject.
            Message getLastMessage(const std::string& stream, const std::string& subject) const;

            // TODO getMessageDitrect ?

            //! Deletes the message at sequence seq in the stream named stream.
            void deleteMessage(const std::string& stream, uint64_t sequence) const;

            //! Similar to deleteMessage except that the content of the deleted message is erased from stable storage.
            void eraseMessage(const std::string& stream, uint64_t sequence) const;

        private:
            std::unique_ptr<MessageManagerPrivate> _impl;
        };
    }
}
