#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <vector>

namespace NatsMq
{
    class ByteArray
    {
    public:
        static ByteArray fromRawData(const char* data, int len);

        ByteArray() = default;

        ByteArray(const char* str);

        ByteArray(const char* data, int len);

        std::size_t size() const;

        std::string toString() const;

        // if ByteArray created used fromRawData then may not have a string null terminating
        const char* constData() const;

        bool operator==(const ByteArray& other) const
        {
            return _bytes == other._bytes;
        }

    private:
        std::vector<char> _bytes;
    };

    struct Options
    {
        std::string name;
        std::string user;
        std::string password;
        std::string token;
        bool        randomize;      ///< default: false
        bool        secure;         ///< default: false
        bool        verbose;        ///< default: false
        bool        pedantic;       ///< default: false
        bool        allowReconnect; ///< default: true
        bool        echo;           ///< default: true
        bool        sendAsap;       ///< default: false
        int64_t     timeout;
        int64_t     pingInterval;
        int64_t     reconnectWait;
        int         maxPingsOut;
        int         ioBufferSize;
        int         maxReconnect;
        int         reconnectBufferSize;
        int         maxPendingMessages;

        Options();
    };

    struct IOStatistic
    {
        uint64_t inMsgs;
        uint64_t inBytes;
        uint64_t outMsgs;
        uint64_t outBytes;
        uint64_t reconnected;
    };

    struct SubscriptionStatistic
    {
        int     pendingMsgs;
        int     pendingBytes;
        int     maxPendingMsgs;
        int     maxPendingBytes;
        int64_t deliveredMsgs;
        int64_t droppedMsgs;
    };

    // --------------------------------- Jet Stream --------------------------------- //

    struct JsOptions
    {
        std::string domain;
        int64_t     timeout{ 5000 };
    };

    struct JsConsumerOptions
    {
        enum class DeliveryPolicy
        {
            DeliverAll = 0,         ///< Starts from the very beginning of a stream. This is the default.
            DeliverLast,            ///< Starts with the last sequence received.
            DeliverNew,             ///< Starts with messages sent after the consumer is created.
            DeliverByStartSequence, ///< Starts from a given sequence.
            DeliverByStartTime,     ///< Starts from a given UTC time (number of nanoseconds since epoch)
            DeliverLastPerSubject,
        };

        enum class AckPolicy
        {
            AckExplicit = 0, ///< Requires ack or nack for all messages.
            AckNone,         ///< Requires no acks for delivered messages.
            AckAll,          ///< When acking a sequence number, this implicitly acks all sequences below this one as well.
        };

        std::string name;
        std::string durableName;
    };

    struct JsStreamConfig
    {
        using Subjects = std::vector<std::string>;

        enum class Storage
        {
            File = 0,
            Memory
        };

        Storage     storage{ Storage::File }; // Невозможно поменять в существующем стриме.
        std::string name;
        Subjects    subjects;
    };

    struct JsPublishAck
    {
        uint64_t    sequence;
        std::string stream;
        std::string domain;
        bool        duplicate;
    };

    struct JsPublishOptions
    {
        std::string msgID;
        std::string expectStream;
        std::string expectLastMessageID;
        uint64_t    expectLastSequence{ 0 };
        uint64_t    expectLastSubjectSequence{ 0 };
        int64_t     timeout{ 2000 };
        bool        expectNoMessage{ false };
    };

    struct KeyValueConfig
    {
        std::string            name;
        uint8_t                history{ 1 };
        std::optional<int32_t> maxValueSize;
        std::optional<int64_t> maxBytes;
    };

    enum class ConnectionStatus
    {
        Disconnected = 0,
        Connecting,
        Connected,
        Closed,
        Reconnecting,
        DrainingSubs,
        DrainingPubs
    };

    enum class Status
    {
        Ok = 0,        ///< Success
        Error,         ///< Generic error
        ProtocolError, ///< Error when parsing a protocol message, or not getting the expected message.

        IOError,     ///< IO Error (network communication).
        LineTooLong, ///< The protocol message read from the socket does not fit in the read buffer.

        ConnectionClosed, ///< Operation on this connection failed because the connection is closed.

        NoServers, ///< Unable to connect, the server could not be reached or is not running.

        StaleConnection, ///< The server closed our connection because it did not receive PINGs at the expected interval.

        SecureConnectionWanted, ///< The client is configured to use TLS, but the server is not.

        SecureConnectionRequired, ///< The server expects a TLS connection.
        ConnectionDisconnected,   ///< The connection was disconnected. Depending on the configuration, the connection may reconnect.

        ConnectionAuthFailed, ///< The connection failed due to authentication error.
        NotPermited,          ///< The action is not permitted.
        NotFound,             ///< An action could not complete because something was not found. So far, this is an internal error.

        AddressMissing, ///< Incorrect URL. For instance no host specified in the URL.

        InvalidSubject, ///< Invalid subject, for instance NULL or empty string.
        InvalidArg,     ///< An invalid argument is passed to a function. For instance passing NULL to an API that does not accept this value.

        InvalidSubscription, ///< The call to a subscription function fails because the subscription has previously been closed.

        InvalidTimeout, ///< Timeout must be positive numbers.

        IllegalState, ///< An unexpected state, for instance calling #natsSubscription_NextMsg() on an asynchronous subscriber.

        SlowConsumer, ///< The maximum number of messages waiting to be delivered has been reached. Messages are dropped.

        MaxPayload, ///< Attempt to send a payload larger than the maximum allowed by the NATS Server.

        MaxDeliveredMsgs, ///< Attempt to receive more messages than allowed, for instance because of #natsSubscription_AutoUnsubscribe().

        InsufficientBuffer, ///< A buffer is not large enough to accommodate the data.

        NoMemmory, ///< An operation could not complete because of insufficient memory.

        SysError, ///< Some system function returned an error.

        Timeout, ///< An operation timed-out. For instance#natsSubscription_NextMsg().

        FailedToInitialize, ///< The library failed to initialize.
        NotInitialize,      ///< The library is not yet initialized.

        SslError, ///< An SSL error occurred when trying to establish a connection.

        NoServerSupport, ///< The server does not support this action.

        NotYetConnected, ///< A connection could not be immediately established and #natsOptions_SetRetryOnFailedConnect() specified
                         ///  a connected callback. The connect is retried asynchronously.

        Draining, ///< A connection and/or subscription entered the draining mode. Some operations will fail when in that mode.

        InvalidQueueName, ///< An invalid queue name was passed when creating a queue subscription.

        NoResponders, ///< No responders were running when the server received the request.

        Mismatch, ///< For JetStream subscriptions, it means that a consumer sequence mismatch was discovered.

        MissedHeartbeat, ///< For JetStream subscriptions, it means that the library detected that server heartbeats have been missed.
    };

    enum class JsError
    {
        NoJsError                   = 999,
        AccountResourcesExceededErr = 10002,       ///< Resource limits exceeded for account
        BadRequestErr,                             ///< Bad request
        ClusterIncompleteErr,                      ///< Incomplete results
        ClusterNoPeersErr,                         ///< No suitable peers for placement
        ClusterNotActiveErr,                       ///< JetStream not in clustered mode
        ClusterNotAssignedErr,                     ///< JetStream cluster not assigned to this server
        ClusterNotAvailErr,                        ///< JetStream system temporarily unavailable
        ClusterNotLeaderErr,                       ///< JetStream cluster can not handle request
        ClusterRequiredErr,                        ///< JetStream clustering support required
        ClusterTagsErr,                            ///< Tags placement not supported for operation
        ConsumerCreateErr,                         ///< General consumer creation failure string
        ConsumerNameExistErr,                      ///< Consumer name already in use
        ConsumerNotFoundErr,                       ///< Consumer not found
        SnapshotDeliverSubjectInvalidErr,          ///< Deliver subject not valid
        ConsumerDurableNameNotInSubjectErr,        ///< Consumer expected to be durable but no durable name set in subject
        ConsumerDurableNameNotMatchSubjectErr,     ///< Consumer name in subject does not match durable name in request
        ConsumerDurableNameNotSetErr,              ///< Consumer expected to be durable but a durable name was not set
        ConsumerEphemeralWithDurableInSubjectErr,  ///< Consumer expected to be ephemeral but detected a durable name set in subject
        ConsumerEphemeralWithDurableNameErr,       ///< Consumer expected to be ephemeral but a durable name was set in request
        StreamExternalApiOverlapErr,               ///< Stream external api prefix must not overlap
        StreamExternalDelPrefixOverlapsErr,        ///< Stream external delivery prefix overlaps with stream subject
        InsufficientResourcesErr,                  ///< Insufficient resources
        StreamInvalidExternalDeliverySubjErr,      ///< Stream external delivery prefix must not contain wildcards
        InvalidJSONErr,                            ///< Invalid JSON
        MaximumConsumersLimitErr,                  ///< Maximum consumers exceeds account limit
        MaximumStreamsLimitErr,                    ///< Maximum number of streams reached
        MemoryResourcesExceededErr,                ///< Insufficient memory resources available
        MirrorConsumerSetupFailedErr,              ///< Generic mirror consumer setup failure
        MirrorMaxMessageSizeTooBigErr,             ///< Stream mirror must have max message size >= source
        MirrorWithSourcesErr,                      ///< Stream mirrors can not also contain other sources
        MirrorWithStartSeqAndTimeErr,              ///< Stream mirrors can not have both start seq and start time configured
        MirrorWithSubjectFiltersErr,               ///< Stream mirrors can not contain filtered subjects
        MirrorWithSubjectsErr,                     ///< Stream mirrors can not also contain subjects
        NoAccountErr,                              ///< Account not found
        ClusterUnSupportFeatureErr,                ///< Not currently supported in clustered mode
        NoMessageFoundErr,                         ///< No message found
        NotEmptyRequestErr,                        ///< Expected an empty request payload
        NotEnabledForAccountErr,                   ///< JetStream not enabled for account
        ClusterPeerNotMemberErr,                   ///< Peer not a member
        RaftGeneralErr,                            ///< General RAFT error
        RestoreSubscribeFailedErr,                 ///< JetStream unable to subscribe to restore snapshot
        SequenceNotFoundErr,                       ///< Sequence not found
        ClusterServerNotMemberErr,                 ///< Server is not a member of the cluster
        SourceConsumerSetupFailedErr,              ///< General source consumer setup failure
        SourceMaxMessageSizeTooBigErr,             ///< Stream source must have max message size >= target
        StorageResourcesExceededErr,               ///< Insufficient storage resources available
        StreamAssignmentErr,                       ///< Generic stream assignment error
        StreamCreateErr,                           ///< Generic stream creation error
        StreamDeleteErr,                           ///< General stream deletion error
        StreamGeneralError,                        ///< General stream failure
        StreamInvalidConfig,                       ///< Stream configuration validation error
        StreamLimitsErr,                           ///< General stream limits exceeded error
        StreamMessageExceedsMaximumErr,            ///< Message size exceeds maximum allowed
        StreamMirrorNotUpdatableErr,               ///< Mirror configuration can not be updated
        StreamMismatchErr,                         ///< Stream name in subject does not match request
        StreamMsgDeleteFailed,                     ///< Generic message deletion failure error
        StreamNameExistErr,                        ///< Stream name already in use
        StreamNotFoundErr,                         ///< Stream not found
        StreamNotMatchErr,                         ///< Expected stream does not match
        StreamReplicasNotUpdatableErr,             ///< Replicas configuration can not be updated
        StreamRestoreErr,                          ///< Restore failed
        StreamSequenceNotMatchErr,                 ///< Expected stream sequence does not match
        StreamSnapshotErr,                         ///< Snapshot failed
        StreamSubjectOverlapErr,                   ///< Subjects overlap with an existing stream
        StreamTemplateCreateErr,                   ///< Generic template creation failed
        StreamTemplateDeleteErr,                   ///< Generic stream template deletion failed error
        StreamTemplateNotFoundErr,                 ///< Template not found
        StreamUpdateErr,                           ///< Generic stream update error
        StreamWrongLastMsgIDErr,                   ///< Wrong last msg ID
        StreamWrongLastSequenceErr,                ///< Wrong last sequence
        TempStorageFailedErr,                      ///< JetStream unable to open temp storage for restore
        TemplateNameNotMatchSubjectErr,            ///< Template name in subject does not match request
        StreamReplicasNotSupportedErr,             ///< Replicas > 1 not supported in non-clustered mode
        PeerRemapErr,                              ///< Peer remap failed
        NotEnabledErr,                             ///< JetStream not enabled
        StreamStoreFailedErr,                      ///< Generic error when storing a message failed
        ConsumerConfigRequiredErr,                 ///< Consumer config required
        ConsumerDeliverToWildcardsErr,             ///< Consumer deliver subject has wildcards
        ConsumerPushMaxWaitingErr,                 ///< Consumer in push mode can not set max waiting
        ConsumerDeliverCycleErr,                   ///< Consumer deliver subject forms a cycle
        ConsumerMaxPendingAckPolicyRequiredErr,    ///< Consumer requires ack policy for max ack pending
        ConsumerSmallHeartbeatErr,                 ///< Consumer idle heartbeat needs to be >= 100ms
        ConsumerPullRequiresAckErr,                ///< Consumer in pull mode requires explicit ack policy
        ConsumerPullNotDurableErr,                 ///< Consumer in pull mode requires a durable name
        ConsumerPullWithRateLimitErr,              ///< Consumer in pull mode can not have rate limit set
        ConsumerMaxWaitingNegativeErr,             ///< Consumer max waiting needs to be positive
        ConsumerHBRequiresPushErr,                 ///< Consumer idle heartbeat requires a push based consumer
        ConsumerFCRequiresPushErr,                 ///< Consumer flow control requires a push based consumer
        ConsumerDirectRequiresPushErr,             ///< Consumer direct requires a push based consumer
        ConsumerDirectRequiresEphemeralErr,        ///< Consumer direct requires an ephemeral consumer
        ConsumerOnMappedErr,                       ///< Consumer direct on a mapped consumer
        ConsumerFilterNotSubsetErr,                ///< Consumer filter subject is not a valid subset of the interest subjects
        ConsumerInvalidPolicyErr,                  ///< Generic delivery policy error
        ConsumerInvalidSamplingErr,                ///< Failed to parse consumer sampling configuration
        StreamInvalidErr,                          ///< Stream not valid
        ConsumerWQRequiresExplicitAckErr = 10098,  ///< Workqueue stream requires explicit ack
        ConsumerWQMultipleUnfilteredErr,           ///< Multiple non-filtered consumers not allowed on workqueue stream
        ConsumerWQConsumerNotUniqueErr,            ///< Filtered consumer not unique on workqueue stream
        ConsumerWQConsumerNotDeliverAllErr,        ///< Consumer must be deliver all on workqueue stream
        ConsumerNameTooLongErr,                    ///< Consumer name is too long
        ConsumerBadDurableNameErr,                 ///< Durable name can not contain '.', '*', '>'
        ConsumerStoreFailedErr,                    ///< Error creating store for consumer
        ConsumerExistingActiveErr,                 ///< Consumer already exists and is still active
        ConsumerReplacementWithDifferentNameErr,   ///< Consumer replacement durable config not the same
        ConsumerDescriptionTooLongErr,             ///< Consumer description is too long
        ConsumerWithFlowControlNeedsHeartbeatsErr, ///< Consumer with flow control also needs heartbeats
        StreamSealedErr,                           ///< Invalid operation on sealed stream
        StreamPurgeFailedErr,                      ///< Generic stream purge failure
        StreamRollupFailedErr,                     ///< Generic stream rollup failure
        ConsumerInvalidDeliverSubjectErr,          ///< Invalid push consumer deliver subject
        StreamMaxBytesRequiredErr,                 ///< Account requires a stream config to have max bytes set
        ConsumerMaxRequestBatchNegativeErr,        ///< Consumer max request batch needs to be > 0
        ConsumerMaxRequestExpiresToSmallErr,       ///< Consumer max request expires needs to be > 1ms
        ConsumerMaxDeliverBackoffErr,              ///< Max deliver is required to be > length of backoff values
        StreamInfoMaxSubjectsErr,                  ///< Subject details would exceed maximum allowed
    };

    struct Message;
    struct IncomingMessage;
    struct JsIncomingMessage;

    using ConnectionStateCb = std::function<void(ConnectionStatus)>;
    using ErrorCb           = std::function<void(Status, const std::string&)>;
    using SubscriptionCb    = std::function<void(const IncomingMessage&)>;
    using JsSubscriptionCb  = std::function<void(const JsIncomingMessage&)>;
    using JsAsyncErrorCb    = std::function<void(const Message&, Status, JsError)>;
}
