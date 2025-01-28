#pragma once
#include <map>
#include <memory>
#include <vector>

#include "Entities.h"
#include "Export.h"

namespace NatsMq
{
    struct Message
    {
        using Headers = std::map<std::string, std::string>;

        Message() = default;

        Message(std::string subj, std::string d)
            : subject(std::move(subj))
        {
            data.insert(data.end(), std::make_move_iterator(d.begin()), std::make_move_iterator(d.end()));
        }

        Message(std::string subj, std::vector<uint8_t> d)
            : subject(std::move(subj))
            , data(std::move(d))
        {
        }

        operator std::string() const
        {
            return std::string(reinterpret_cast<const char*>(data.data()), data.size());
        }

        std::string          subject;      ///< Main message subject
        std::string          replySubject; ///< Subject for reply on request
        std::vector<uint8_t> data;         ///< Raw data
        Headers              headers;
    };

    namespace Js
    {
        struct IncomingMessagePrivate;

        struct NATSMQ_EXPORT IncomingMessage
        {
        private:
            std::unique_ptr<IncomingMessagePrivate> _impl;

        public:
            IncomingMessage(IncomingMessagePrivate* impl);

            ~IncomingMessage();

            IncomingMessage(IncomingMessage&&);

            IncomingMessage& operator=(IncomingMessage&&);

            operator std::string() const;

            /// Tells the server that the message was successfully processed and it can move on to the next message.
            void ack() const;

            /// Synchronous version of ack(). Indicates successful message processing, and waits for confirmation from the server that the acknowledgment has been processed.
            void ackSync() const;

            /// Tells the server to redeliver the message. You can configure the number of redeliveries by passing MaxDeliver when you subscribe.
            /// @arg delayMs - 	the amount of time before the redelivery expressed in milliseconds. if the value is less than 0, then redeliver occurs immediately
            void nak(uint64_t delayMs = -1) const;

            /// Tells the server that this message is being worked on. It resets the redelivery timer on the server.
            void inProgress() const;

            /// Tells the server to not redeliver this message, regardless of the value MaxDeliver
            void terminate() const;

            /// Returns the sequence number of this JetStream message
            uint64_t sequnce() const noexcept;

            /// Returns the timestamp (in UTC) of this JetStream message
            int64_t timestamp() const noexcept;

            /// Returns meta info about message
            MessageMeta meta() const;

            Message msg;
        };
    }
}
