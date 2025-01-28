#include "Publisher.h"

#include "Exceptions.h"
#include "Message.h"
#include "private/utils.h"

using namespace NatsMq::Js;

namespace
{
    using JsPubAckPtr = std::unique_ptr<jsPubAck, decltype(&jsPubAck_Destroy)>;

    jsPubOptions toCnatsPublishOptions(const NatsMq::Js::PublishOptions& options)
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

    NatsMq::Js::PublishAck toPublishAck(jsPubAck* ack)
    {
        NatsMq::Js::PublishAck result;

        result.stream    = NatsMq::emptyStringIfNull(ack->Stream);
        result.domain    = NatsMq::emptyStringIfNull(ack->Domain);
        result.sequence  = ack->Sequence;
        result.duplicate = ack->Duplicate;

        return result;
    }

    NatsMq::NatsMsgPtr createNatsMessageWithSwapException(const NatsMq::Message& msg)
    {
        try
        {
            return createCnatsMessage(msg);
        }
        catch (const NatsMq::Exception& exc)
        {
            throw NatsMq::JsException(exc.status, NatsMq::Js::Status::NoJsError);
        }
    }
}

Publisher::Publisher(jsCtx* context)
    : _context(context)
{
}

PublishAck Publisher::publish(Message msg, PublishOptions options) const
{
    auto natsOptions = toCnatsPublishOptions(options);
    auto natsMsg     = createNatsMessageWithSwapException(msg);
    return makePublish(natsMsg.get(), &natsOptions);
}

void Publisher::apublish(Message msg, PublishOptions options) const
{
    auto natsOptions = toCnatsPublishOptions(options);
    auto natsMsg     = createNatsMessageWithSwapException(msg);
    makeAsyncPublish(natsMsg.get(), &natsOptions);
    natsMsg.release();
}

void Publisher::waitAsyncPublishComplete(int64_t timeoutMs) const
{
    jsPubOptions natsOptions;
    jsPubOptions_Init(&natsOptions);
    natsOptions.MaxWait = timeoutMs;
    jsExceptionIfError(js_PublishAsyncComplete(_context, timeoutMs < 0 ? nullptr : &natsOptions));
}

std::vector<NatsMq::Message> Publisher::asyncPendingMessages() const
{
    natsMsgList pending;
    jsExceptionIfError(js_PublishAsyncGetPendingList(&pending, _context));

    std::vector<Message> msgs;
    for (auto i = 0; i < pending.Count; ++i)
        msgs.push_back(fromCnatsMessage(pending.Msgs[i]));

    natsMsgList_Destroy(&pending);

    return msgs;
}

PublishAck Publisher::makePublish(natsMsg* msg, jsPubOptions* options) const
{
    jsErrCode jsErr{ jsErrCode(0) };
    jsPubAck* rawAck{ nullptr };

    const auto  status = js_PublishMsg(&rawAck, _context, msg, options, &jsErr);
    JsPubAckPtr ack{ rawAck, &jsPubAck_Destroy };

    jsExceptionIfError(status, jsErr);
    return toPublishAck(rawAck);
}

void Publisher::makeAsyncPublish(natsMsg* msg, jsPubOptions* options) const
{
    jsExceptionIfError(js_PublishMsgAsync(_context, &msg, options));
}
