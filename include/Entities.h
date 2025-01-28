#pragma once
#include <cstdint>
#include <functional>
#include <string>

namespace NatsMq
{
    struct Message;

    namespace Js
    {
        struct IncomingMessage;
    }

    struct UserCredentials
    {
        std::string login;
        std::string password;
    };

    struct ConnectionOptions
    {
        bool randomize{ false }; ///< server urls list is formed in random order
        // Secure,                  ///< bool // TODO
        bool            verbose{ false };                 ///< Sets the verbose mode. If true, sends are echoed by the server with an OK protocol message.
        bool            pedanic{ false };                 ///< Sets the pedantic mode. If true some extra checks will be performed by the server.
        bool            allowRecconect{ true };           ///< Specifies whether or not the client library should try to reconnect when losing the connection to the NATS Server.
        bool            retryOnFailedConnect{ true };     ///< If retry is set to true and connection cannot be established right away, the library will attempt to connect based on the reconnect attempts and delay settings.
        int             maxReconnect{ 60 };               ///< Specifies the maximum number of reconnect attempts.
        bool            echo{ true };                     ///< This configures whether the server will echo back messages that are sent on this connection if there is also matching subscriptions.
        bool            sendAsap{ false };                ///< For performance, the library triesto buffer as much data as possible before sending. This option to will make Publish send the data right away
        bool            disableNoResponders{ false };     ///< Disable no responder error
        bool            useGlobalMsgDelivery{ true };     ///< Async subs from this connection will use a shared thread pool responsible for message delivery. See more: http://nats-io.github.io/nats.c/group__opts_group.html#gabf060c92648b50c069f0abe7cbb06f1c
        bool            failRequestOnDisconnect{ false }; ///< If this option is enabled, all pending request() family calls will fail with the NATS_CONNECTION_DISCONNECTED status.
        int             maxPingsOut{ 2 };                 ///< Specifies the maximum number of PINGs without corresponding PONGs
        int             IOBufferSize{ 32768 };            ///< Sets the size, in bytes, of the internal read/write buffers used for reading/writing data from a socket. If not specified, or the value is 0, the library will use a default value, currently set to 32KB.
        int             reconnectBufferSize{ 8388608 };   ///< Sets the size, in bytes, of the backing buffer holding published data while the library is reconnecting.
        int             maxPendingMessages{ 65536 };      ///< Specifies the maximum number of inbound messages that can be buffered in the library, for each subscription, before inbound messages are dropped and NATS_SLOW_CONSUMER status is reported to the natsErrHandler callback (if one has been set).
        long long       timeout{ 2000 };                  ///< This timeout, expressed in milliseconds, is used to interrupt a (re)connect attempt to a NATS Server.
        long long       pingInterval{ 120000 };           ///< Interval, expressed in milliseconds, in which the client sends PING protocols to the NATS Server.
        long long       reconnectWait{ 2000 };            ///< Specifies how long to wait between two reconnect attempts from the same server.
        std::string     name;                             ///< This name is sent as part of the CONNECT protocol. There is no default name.
        std::string     token;                            ///< Tokens are usually provided through the URL in the form: nats://mytoken@localhost:4222. see more http://nats-io.github.io/nats.c/group__opts_group.html#gad58a5b9dabadeebda30e952ff7b39193
        UserCredentials userCreds;                        ///< Credentials are usually provided through the URL in the form: nats://foo:bar@localhost:4222. see more http://nats-io.github.io/nats.c/group__opts_group.html#ga5b99da7dd74aac3be962f323c3863d9e
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

    namespace Js
    {
        struct Options
        {
            struct Stream
            {
                struct Purge
                {
                    std::string subject;       ///< This is the subject to match against messages for the purge command.
                    uint64_t    sequence{ 0 }; ///< Purge up to but not including sequence.
                    uint64_t    keep{ 0 };     ///< Number of messages to keep.
                } purge;

                struct Info
                {
                    bool        DeletedDetails{ false }; ///< Get the list of deleted message sequences.
                    std::string SubjectsFilter;          ///< Get the list of subjects in this stream.
                } info;

            } stream; ///< Optional stream options.

            struct PublishAsync
            {
                int64_t maxPending{ -1 }; ///< Maximum outstanding asynchronous publishes that can be inflight at one time.
                int64_t stallWait{ 200 }; ///< Amount of time (in milliseconds) to wait in a PublishAsync call when there is MaxPending inflight messages.
            } publishAsync;

            std::string prefix{ "$JS.API" }; ///< JetStream prefix, default is "$JS.API"
            std::string domain;              ///< Domain changes the domain part of JetSteam API prefix.
            int64_t     timeout{ 5000 };     ///< Amount of time (in milliseconds) to wait for various JetStream API requests, default is 5000 ms (5 seconds).
        };

        enum class StorageType
        {
            File = 0,
            Memory
        };

        enum class DiscardPolicy
        {
            RemoveOlderMessages = 0,
            FailStoreMessage
        };

        enum class RetentionPolicy
        {
            Limits = 0, ///< Specifies that messages are retained until any given limit is reached, which could be one of MaxMsgs, MaxBytes, or MaxAge
            Interest,   ///< Specifies that when all known observables have acknowledged a message it can be removed.
            WorkQueue   ///< Specifies that when the first worker or subscriber acknowledges the message it can be removed
        };

        enum class StorageCompression
        {
            None = 0,
            Compression32
        };

        struct Republish
        {
            bool        headersOnly{ false };
            std::string source;
            std::string destination;
        };

        //! more info https://docs.nats.io/nats-concepts/jetstream/streams
        struct StreamConfig
        {
            using Subjects = std::vector<std::string>;

            // Name cannot contain whitespace, ., *, >, path separators (forward or backwards slash), and non-printable characters.
            std::string name;        ///< Unique stream identifies. Can't change in existing stream
            std::string description; ///< Stream description
            Subjects    subjects;    ///< A list of subjects to bind. Wildcards are supported. Cannot be set for mirror streams
            //std::string templates;

            bool allowDirect{ false };  ///< ?
            bool allowRollup{ false };  ///< Allows messages to be placed into the system and purge all older messages using a special message header
            bool mirrorDirect{ false }; ///< ?
            bool noAck{ false };        ///< Disables acknowledging messages that are received by the Stream.
            bool sealed{ false };       ///< Seal a stream so no messages can get our or in.
            bool denyDelete{ false };   ///< Restrict the ability to delete messages.
            bool denyPurge{ false };    ///< Restrict the ability to purge messages.

            Republish republish; ///< Allows a source subject to be mapped to a destination subject for republishing.

            StorageType        storage{ StorageType::File };                  ///< The storage type for stream data. Can't change in existing stream
            DiscardPolicy      discard{ DiscardPolicy::RemoveOlderMessages }; ///< The behavior of discarding messages when any streams’ limits have been reached.
            RetentionPolicy    retention{ RetentionPolicy::Limits };          ///< Declares the retention policy for the stream.
            StorageCompression compression{ StorageCompression::None };       ///< Storage compression policy

            // placement
            // mirror
            // sources

            int64_t maxMessages{ -1 };           ///< Maximum number of messages stored in the stream. Adheres to Discard Policy
            int64_t maxMessagesPerSubject{ -1 }; ///< Max messages in each subject
            int32_t maxMessageSize{ -1 };        ///< The largest message that will be accepted by the Stream. The size of a message is a sum of payload and headers.
            int64_t maxAge{ 0 };                 ///< Maximum age of any message in the Stream. in Seconds
            int64_t maxBytes{ -1 };              ///< Maximum number of bytes stored in the stream. Adheres to Discard Policy
            int64_t maxConsumers{ -1 };          ///< Maximum number of Consumers that can be defined for a given Stream, -1 for unlimited.
            int64_t replicas{ 1 };               ///< How many replicas to keep for each message in a clustered JetStream, maximum 5.
            int64_t duplicateWindow{ 0 };        ///< The window within which to track duplicate messages, expressed in nanoseconds.
        };

        struct StreamState
        {
            struct LostData
            {
                std::vector<uint64_t> messages;
                uint64_t              bytes;
            };

            struct SubjectState
            {
                std::string subject;
                uint64_t    messages;
            };

            uint64_t messages;
            uint64_t bytes;

            uint64_t firstSequence;
            int64_t  firstTime; ///< UTC time expressed as number of nanoseconds since epoch.
            uint64_t lastSequence;
            int64_t  lastTime;

            std::vector<SubjectState> subjects;
            std::vector<uint64_t>     deleted;

            LostData lostData;
            int64_t  consumers;
        };

        struct StreamCluster
        {
            struct Replica
            {
                bool        current;
                bool        offline;
                std::string name;
                int64_t     active;
                uint64_t    lag;
            };

            std::string name;
            std::string leader;

            std::vector<Replica> replicas;
        };

        struct ExternalStream
        {
            std::string APIPrefix;
            std::string deliverPrefix;
        };

        struct SubjectTransform
        {
            std::string source;
            std::string destination;
        };

        struct StreamSourceInfo
        {
            std::string    name;
            ExternalStream external;
            uint64_t       lag;
            int64_t        active;
            std::string    filterSubject;

            std::vector<SubjectTransform> subjectTransforms;
        };

        struct StreamAlternate
        {
            std::string name;
            std::string domain;
            std::string cluster;
        };

        struct StreamInfo
        {
            StreamConfig     config;
            int64_t          createdNs; ///< UTC time expressed as number of nanoseconds since epoch.
            StreamState      state;
            StreamCluster    cluster;
            StreamSourceInfo mirror;

            std::vector<StreamSourceInfo> sources;
            std::vector<StreamAlternate>  alternates;
        };

        struct PublishAck
        {
            uint64_t    sequence;
            std::string stream;
            std::string domain;
            bool        duplicate;
        };

        struct PublishOptions
        {
            std::string msgID;
            std::string expectStream;
            std::string expectLastMessageID;
            uint64_t    expectLastSequence{ 0 };
            uint64_t    expectLastSubjectSequence{ 0 };
            int64_t     timeout{ 2000 };
            bool        expectNoMessage{ false };
        };

        struct MessageMeta
        {
            struct SequencePair
            {
                uint64_t consumer;
                uint64_t stream;
            };

            uint64_t    delivered;
            uint64_t    pending;
            int64_t     timestamp;
            std::string stream;
            std::string consumer;
            std::string domain;

            SequencePair sequence;
        };

        /// Represents a consumer sequence mismatch between the server and client views.
        struct SubscriptionMismatch
        {
            uint64_t stream; ///< This is the stream sequence that the application should resume from.
            uint64_t client; ///< This is the consumer sequence that was last received by the library.
            uint64_t server; ///< This is the consumer sequence last sent by the server.
        };

        struct SequnceInfo
        {
            uint64_t consumer;
            uint64_t stream;
            int64_t  last;
        };

        //! more info https://docs.nats.io/nats-concepts/jetstream/consumers
        struct ConsumerConfig
        {
            enum class DeliverPolicy
            {
                NotSet = -1,
                All    = 0,      ///< Starts from the very beginning of a stream. This is the default.
                Last,            ///< Starts with the last sequence received.
                New,             ///< Starts with messages sent after the consumer is created.
                ByStartSequence, ///< Starts from a given sequence.
                ByStartTime,     ///< Starts from a given UTC time (number of nanoseconds since epoch)
                LastPerSubject   ///< Starts with the last message for all subjects received.
            };

            enum class AckPolicy
            {
                NotSet   = -1,
                Explicit = 0, ///< Requires ack or nack for all messages.
                None,         ///< Requires no acks for delivered messages.
                All           ///< When acking a sequence number, this implicitly acks all sequences below this one as well
            };

            enum class ReplayPolicy
            {
                NotSet  = -1,
                Instant = 0, ///< Replays messages as fast as possible.
                Original     ///< Maintains the same timing as the messages were received.
            };

            std::string name;
            std::string durable; ///< If set, clients can have subscriptions bind to the consumer and resume until the consumer is explicitly deleted.
            std::string description;
            std::string filterSubject;  ///< A subject that overlaps with the subjects bound to the stream to filter delivery to subscribers
            std::string sampleFrequncy; ///< Sets the percentage of acknowledgments that should be sampled for observability, 0-100.

            std::string deliverSubject; ///< The subject to deliver messages to. Setting this field decides whether the consumer is push or pull-based.
            std::string deliverGroup;   ///< The queue group name used to distribute messages among subscribers.

            DeliverPolicy deliverPolicy{ DeliverPolicy::NotSet };
            AckPolicy     ackPolicy{ AckPolicy::NotSet };
            ReplayPolicy  replayPolicy{ ReplayPolicy::NotSet };

            uint64_t optStartSeq{ 0 };        ///< Used with the DeliverByStartSequence deliver policy.
            int64_t  optStartTime{ 0 };       ///< Used with the DeliverByStartTime deliver policy.
            int64_t  ackWait{ 0 };            ///<
            int64_t  maxDeliver{ 0 };         ///< he maximum number of times a specific message delivery will be attempted.
            int64_t  maxAckPending{ 0 };      ///< Defines the maximum number of messages, without acknowledgment, that can be outstanding.
            int64_t  maxWaiting{ 0 };         ///< The maximum number of waiting pull requests
            uint64_t rateLimit{ 0 };          ///< Throttles the delivery of messages to the consumer, in bits per second.
            int64_t  heartbeat{ 0 };          ///< If set, the server will regularly send a status message to the client during inactivity, indicating that the JetStream service is up and running.
            int64_t  maxRequestBatch{ 0 };    ///< The maximum batch size a single pull request can make.
            int64_t  maxRequestExpires{ 0 };  ///< The maximum duration a single pull request will wait for messages to be available to pull.
            int64_t  maxRequestMaxBytes{ 0 }; ///< The maximum total bytes that can be requested in a given batch.
            int64_t  inactiveTreshold{ 0 };   ///< Duration that instructs the server to clean up consumers inactive for that long.
            int64_t  replicas{ 0 };           ///< Sets the number of replicas for the consumer's state.

            std::vector<int64_t> backOff; ///< A sequence of durations controlling the redelivery of messages on nak or acknowledgment timeout.
            // TODO Пока не придумал как удобно освобождать память, т.к.
            // функция cnatc библиотеки ждет const char**
            //std::vector<std::string> filterSubjects; ///< A set of subjects that overlap with the subjects bound to the stream to filter delivery to subscribers.
            //std::vector<std::string> userMetadata; ///< A set of application-defined key-value pairs for associating metadata with the consumer

            bool flowControl{ false };   ///< Enables per-subscription flow control using a sliding-window protocol.
            bool headersOnly{ false };   ///< Delivers only the headers of messages in the stream
            bool memoryStorage{ false }; ///< If set, forces the consumer state to be kept in memory rather than inherit the storage type of the stream
        };

        struct Consumer
        {
            std::string stream;
            std::string name;
            int64_t     created;

            ConsumerConfig config;

            SequnceInfo delivered;
            SequnceInfo ackFloor;

            int64_t  ackPendingCount;
            int64_t  redeliveredCount;
            int64_t  waitingCount;
            uint64_t pendingCount;

            StreamCluster cluster;

            bool pushBounds;
        };

        struct SubscriptionOptions
        {
            bool manualAck{ false };
            bool ordered{ false };

            std::string stream;
            std::string consumer;
            std::string queue;

            ConsumerConfig config;
        };

        struct StreamSource
        {
            std::string    name;
            std::string    filterSubject;
            std::string    domain;
            uint64_t       optStartSequence{ 0 };
            int64_t        optStartTime{ 0 };
            ExternalStream external;
        };

        namespace KeyValue
        {
            struct Config
            {
                std::string  bucket;
                std::string  description;
                int32_t      maxValueSize{ 0 };
                uint8_t      history{ 0 };
                int64_t      ttl{ 0 };
                int64_t      maxBytes{ 0 };
                StorageType  storageType{ 0 };
                int          replicas{ 0 };
                Republish    republish;
                StreamSource mirror;

                std::vector<StreamSource> sources;
            };

            struct Info
            {
                std::string bucket;
                uint64_t    values;
                int64_t     history;
                int64_t     ttl;
                int64_t     replicas;
                uint64_t    bytes;
            };

            struct Element
            {
                std::string          bucket;   ///< name of the bucket
                std::string          key;      ///< key that was retrieved
                std::vector<uint8_t> value;    ///< value for this key without NULL terminate character
                uint64_t             revision; ///< unique sequence for this value
                int64_t              created;  ///< time (in UTC) the data was put in the bucket,
                int64_t              delta;    ///< distance from the latest value
                // ? TODO operation type. It is need?
            };
        }

        struct ObjectStoreConfig
        {
            std::string bucket;
            std::string description;
            int64_t     maxBytes{ -1 };
            int64_t     maxAge{ -1 };
            StorageType storage{ StorageType::File };
            int64_t     replicas{ 1 };
        };

        struct ObjectInfo
        {
            std::string name;
            std::string bucket;
            std::string uid;
            int64_t     size;
            int64_t     chunks;
            bool        deleted{ false };
        };

        struct ObjectElement
        {
            ObjectInfo meta;

            std::vector<uint8_t> data;
        };

        enum class Status
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

        using PublishErrorCb = std::function<void(Message, NatsMq::Status, NatsMq::Js::Status)>;
        using ObjectWatchCb  = std::function<void(ObjectInfo)>;
    }

    using ConnectionStateCb = std::function<void(ConnectionStatus)>;
    using ErrorCb           = std::function<void(Status, const std::string&)>;
    using SubscriptionCb    = std::function<void(Message)>;
    using JsSubscriptionCb  = std::function<void(Js::IncomingMessage)>;
}
