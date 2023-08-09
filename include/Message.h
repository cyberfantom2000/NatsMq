#pragma once

#include <memory>
#include <unordered_map>

#include "Entities.h"

namespace NatsMq
{
    struct Message
    {
        using Headers = std::unordered_multimap<std::string, std::string>;

        Message() = default;

        Message(const std::string& inSubject, const ByteArray& inData)
            : subject(inSubject)
            , data(inData)
        {
        }

        std::string subject;
        std::string reply;
        ByteArray   data;
        Headers     headers;
    };

    struct IncomingMessageImpl;

    struct IncomingMessage : Message
    {
        IncomingMessage() = default;

        IncomingMessage(const std::string& inSubject, const ByteArray& inData)
            : Message(inSubject, inData)
        {
        }

        IncomingMessage(IncomingMessageImpl* msg);
    };

    struct JsIncomingMessageImpl;

    struct JsIncomingMessage : IncomingMessage
    {
        JsIncomingMessage(JsIncomingMessageImpl* msg);

        ~JsIncomingMessage();

        JsIncomingMessage(JsIncomingMessage&&);

        JsIncomingMessage& operator=(JsIncomingMessage&&);

        void ack() const;

        void nack(uint64_t delay = -1) const;

        void inProgress() const;

        void terminate() const;

    private:
        std::unique_ptr<JsIncomingMessageImpl> _impl;
    };
}
