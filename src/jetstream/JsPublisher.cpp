#include "JsPublisher.h"

#include "Exceptions.h"
#include "Utils.h"

namespace
{
    jsPubOptions toCnatsPublishOptions(const NatsMq::JsPublishOptions& options)
    {
        jsPubOptions natsOptions;

        jsPubOptions_Init(&natsOptions);

        if (options.timeout >= 0)
            natsOptions.MaxWait = options.timeout;

        std::string msgID;
        if (options.msgID.size())
            natsOptions.MsgId = options.msgID.c_str();

        std::string expectStream;
        if (options.expectStream.size())
            natsOptions.ExpectStream = options.expectStream.c_str();

        std::string expectLastMessageID;
        if (options.expectLastMessageID.size())
            natsOptions.ExpectLastMsgId = options.expectLastMessageID.c_str();

        natsOptions.ExpectLastSeq        = options.expectLastSequence;
        natsOptions.ExpectLastSubjectSeq = options.expectLastSubjectSequence;
        natsOptions.ExpectNoMessage      = options.expectNoMessage;

        return natsOptions;
    }

    NatsMq::JsPublishAck toPublishAck(jsPubAck* ack)
    {
        NatsMq::JsPublishAck result;

        result.stream    = NatsMq::emptyStringIfNull(ack->Stream);
        result.domain    = NatsMq::emptyStringIfNull(ack->Domain);
        result.sequence  = ack->Sequence;
        result.duplicate = ack->Duplicate;

        return result;
    }

    using JsPubAckPtr = std::unique_ptr<jsPubAck, decltype(&jsPubAck_Destroy)>;

    NatsMq::NatsMsgPtr createNatsMessageWithSwapException(const NatsMq::Message& msg)
    {
        try
        {
            return createNatsMessage(msg);
        }
        catch (const NatsMq::Exception& exc)
        {
            throw NatsMq::JsException(exc.status, NatsMq::JsError::NoJsError);
        }
    }
}

NatsMq::JsPublisher::JsPublisher(jsCtx* context)
    : _context(context)
{
}

NatsMq::JsPublishAck NatsMq::JsPublisher::publish(const Message& msg, const JsPublishOptions& options) const
{
    auto natsOptions = toCnatsPublishOptions(options);
    auto natsMsg     = createNatsMessageWithSwapException(msg);
    return makePublish(natsMsg.get(), &natsOptions);
}

NatsMq::JsPublishAck NatsMq::JsPublisher::asyncPublish(const Message& msg, const JsPublishOptions& options) const
{
    auto natsOptions = toCnatsPublishOptions(options);
    return makeAsyncPublish(msg, &natsOptions);
}

void NatsMq::JsPublisher::waitPublishCompleted(int64_t timeout) const
{
    jsPubOptions natsOptions;
    jsPubOptions_Init(&natsOptions);
    natsOptions.MaxWait = timeout;
    jsExceptionIfError(js_PublishAsyncComplete(_context, timeout < 0 ? nullptr : &natsOptions));
}

std::vector<NatsMq::Message> NatsMq::JsPublisher::getAsyncPendingMessages() const
{
    natsMsgList pending;
    jsExceptionIfError(js_PublishAsyncGetPendingList(&pending, _context));

    std::vector<Message> msgs;
    for (auto i = 0; i < pending.Count; ++i)
        msgs.push_back(fromCnatsMessage(pending.Msgs[i]));

    natsMsgList_Destroy(&pending);

    return msgs;
}

NatsMq::JsPublishAck NatsMq::JsPublisher::makePublish(natsMsg* msg, jsPubOptions* options) const
{
    jsErrCode jsErr{ jsErrCode(0) };
    jsPubAck* rawAck{ nullptr };

    const auto  status = js_PublishMsg(&rawAck, _context, msg, options, &jsErr);
    JsPubAckPtr ack{ rawAck, &jsPubAck_Destroy };

    jsExceptionIfError(status, jsErr);
    return toPublishAck(rawAck);
}

NatsMq::JsPublishAck NatsMq::JsPublisher::makeAsyncPublish(const Message& msg, jsPubOptions* options) const
{
    auto natsMsgPtr = createNatsMessageWithSwapException(msg);
    auto natsMsgRaw = natsMsgPtr.get();
    jsExceptionIfError(js_PublishMsgAsync(_context, &natsMsgRaw, options));
}
