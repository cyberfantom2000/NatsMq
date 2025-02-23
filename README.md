# NatsMq

Cpp wrapper over the [official cnats library](https://github.com/nats-io/nats.c).

- [Installing](#installing)
- [Tests](#tests)
- [Examples](#examples)
    * [Core](#core)
       * [Create core client](#create-core-client)
       * [Publish](#publish)
       * [Subscribe](#subscribe)
       * [Request](#request)
       * [Reply](#reply)
    * [JetStream](#jetstream)
       * [Create JetStream client](#create-jetstream-client)
       * [Jet Stream publish](#jet-stream-publish)
       * [Jet Stream subscribe](#jet-stream-subscribe)
    * [Key Value store](#key-value-store)
        * [Key value store managment](#key-value-store-managment)
        * [Key value store put data](#key-value-store-set-data)
        * [Key value store get data](#key-value-store-get-data)
        * [Key value store remove data](#key-value-store-get-data)
    * [Object store](#object-store)
        * [Object store managment](#object-store-managment)
        * [Object store put data](#object-store-set-data)
        * [Object store get data](#object-store-get-data)
        * [Object store remove data](#object-store-get-data)
    * [Advanced settings](#advanced-settings)

## Installing
You can install library in your project use Fetch Content:
```
include(FetchContent)

FetchContent_Declare(
  natsmq
  GIT_TAG        main
  GIT_REPOSITORY https://gitlab.stc-spb.ru/labDSP/projects/common/natsmq.git
  GIT_SHALLOW    ON
)

set(NATSMQ_BUILD_TESTS OFF)
set(NATS_BUILD_EXAMPLES OFF)

FetchContent_MakeAvailable(natsmq)

target_link_libraries(${PROJECT_NAME} NatsMq)
```
You can also add the project as a submodule or by any other means.
On windows has a dependency on the winsock32 library. For build test required Qt5::Core and Qt5::Network.

By default output files are stored in paths:
```
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
```
Don't forget to define them in your project.

## Tests
To run the tests you will need to install server nats, python3 and Qt5 library. I recomend using [docker image](https://hub.docker.com/_/nats) for server. [Python3](https://www.python.org/downloads/) used for the second command line nats client and use only standart python libraries. Also test project used [GTEST framework](https://github.com/google/googletest). But this will be picked up automatically with cmake FetchContent.
Start the NATS server first, then run a tests. For a JetStream tests, the server must be started with the flag ```--jetstream```.

## Examples
Please note, there may be typos or errors, for specific examples, look in the examples folder.

## Core

### Create core client
For create client you can use static function ```NatsMq::Client::create()```. This function return pointer on a client and you must take care about of freeing the memory. By default, each asynchronous subscriber that is created has its own message delivery thread. To set the size of the thread pool for delivering messages, use the static function. [More information about thread pool size](http://nats-io.github.io/nats.c/group__library_group.html#gab21e94ce7a7d226611ea2c05914cf19d).
Most functions throw exceptions when an error occurs, so you must handle them. All exceptions inherited from std::runtime_error, they have a "what()" method and a public "status" field that stores error code. 
when the client is destroyed, it will close the connection to the server.

```
#include <NatsMq>
#include <memory>
#include <iostream>

using namespace NatsMq;

int main()
{
  std::unique_ptr<Client> client(Client::create());
  try
  {
      client->connect({"nats://localhost:4222"});
  }
  catch(const NatsMq::Exception& exc)
  {
      std::cout << exc.what();
  }
  return 0;
}
```
### Publish
In order not to copy the code, let's pretend that we continue the section [Create core client](#create-core-client).
```
//same as in the previous example

try
{
   client->publish(Message("my_subject", "my_data"));
}
catch(const NatsMq::Exception& exc)
{
   std::cout << exc.what();
}

```
Everything is simple here

### Subscribe
Calling ```subscribe``` method on client you will get Subscription object. You must pass message handler to ```subscribe``` method. Please note that when the object is destroyed, an unsubscribe from the topic will occur.

Expect callback with ```void(*)(NatsMq::Message)``` signathure. The passed callback will be moved to object if is rvalue and copy and move if is lvalue.
```
try
{
    auto cb = [](Message msg){
       std::cout << "Subject: " << msg.subject << "; Data: " << std::string(msg)  << std::endl;
    };

   std::unique_ptr<Subscription> sub(client->subscribe("interesting_subject", std::move(cb)));

   // Wait subscription data
   ...
   //

   NatsMq::SubscriptionStatistic stats = sub.statistics();

   std::cout << "Pending messages: " << stats.pendingMsgs << "\n"
             << "Pending  bytes: " << stats.pendingBytes << "\n"
             << "Max pending messages: " << stats.maxPendingMsgs << "\n"
             << "Max pending bytes: " << stats.maxPendingBytes << "\n"
             << "Delivered messages: " << stats.deliveredMsgs << "\n"
             << "Dropped messages: " << stats.droppedMsgs << std::endl;
}
catch(const NatsMq::Exception& exc)
{
   std::cout << exc.what();
}
```

### Request
You can use two types of requests: synchronous and asynchronous.
A synchronous request will throw a timeout exception if it does not receive data. An asynchronous request will return an ```std::future``` object, from which you can request data when they are needed.
```
using FutureMsg = std::future<NatsMq::Message>;

try
{
    int timeoutMs = 2000;
    Message msg = client->request(Message("req_subject", "req_data"), timeoutMs);
    ...

    FutureMsg futureMsg = client->asyncReuest(Message("req_subject", "req_data"), timeoutMs);
    msg = futureMsg.get(); // wait std::future result or exception
    ...
}
catch(const NatsMq::Exception& exc)
{
}
```

### Reply
To create a replier, recommended enable the sendAsap option to true. This will force the cnats library to send data immediately without caching it.
The response to the request consists of two parts. 1. You create a subscription to the topic you are going to make respond. 2. When there is a new message in this subscription, you reply with the incoming message, but replace the data field.
```
#include <NatsMq>
#include <memory>
#include <iostream>

using namespace NatsMq;

int main()
{
  
  constexpr auto subject = "example_subject";

    std::unique_ptr<Client> client(Client::create());

    try
    {
        ConnectionOptions options;
        options.sendAsap = true; // sending messages without delay

        client->connect({ "nats://172.20.73.29:4222" });

        auto replyCb = [&client](Message msg) {
            /* handle incoming msg */
            std::cout << std::string(msg) << std::endl;

            /* make reply */
            std::string reply = "it is request callback reply";

            // Subject must be replaced by replySubject
            msg.subject = msg.replySubject;
            msg.replySubject.clear();
            msg.data.clear();
            msg.data.insert(msg.data.end(), reply.begin(), reply.end());

            try
            {
                client->publish(std::move(msg));
            }
            catch (const Exception& exc)
            {
                std::cout << exc.what();
            }
        };

        std::unique_ptr<Subscription> sub(client->subscribe(subject, std::move(replyCb)));

        auto reply = client->request(Message(subject, "it is request callback data"));
        std::cout << std::string(reply) << std::endl;
    }
    catch (const NatsMq::Exception& exc)
    {
        std::cout << exc.what();
    }

    return 0;
}
```

## JetStream

### Create JetStream client
For create JetStream client you must first create and connect core client. After you get the JetStream from the core client, it can be removed. This will not close the connection because it is shared between them.
Before publishing, you need to make sure that a stream listening to your subjects exists. Otherwise you will get an exception. You may used 2 function to get or create stream: ```getStream``` or ```getOrCreateStream```.

```
#include <NatsMq>
#include <iostream>
#include <memory>

using namespace NatsMq;

namespace
{
    // JsStream not remove the stream on the server
    auto streamDeleter = [](Js::Stream* stream) {stream->remove(); delete stream; };
    using JsStreamPtr  = std::unique_ptr<Js::Stream, decltype(streamDeleter)>;
}
int main()
{
    std::unique_ptr<Client> client(Client::create());
    
    try
    {
        client->connect({ "nats://localhost:4222" });
        std::unique_ptr<JetStream> js(client->jetstream());

        Js::StreamConfig config;
        config.name     = "my_stream";
        config.storage  = Js::StorageType::Memory;
        config.subjects = { "subject1", "subject2" }; // stream listen this subjects

        // stream cannot be used after destroying js object       
        JsStreamPtr stream1(js->getOrCreateStream(config), streamDeleter);

        // it may not exist
        JsStreamPtr stream2(js->getStream("stream_name2"), streamDeleter);
        if(stream2->exists())
            std::cout << "Stream 2 exists!";
        else
            std::cout << "Whoops stream 2 not exists!";
    }
    catch(const NatsMq::JsException& jsExc)
    {
        std::cout << jsExc.what();
    }
    catch(const NatsMq::Exception& exc)
    {
        std::cout << exc.what();
    }
    return 0;
}
```

### Jet Stream publish
No different than publishing with core. But there are two ways to publish synchronous and asynchronous. With the synchronous method, the thread is blocked until an acknowledgment of receipt of the message is received. An asynchronous call does not block the thread. Instead, if the message has not been acknowledged, an error handler will be called. To react to this you need to register a handler.

```
// just like previous example

try
{
    Js::StreamConfig config;
    config.name     = "my_stream";
    config.storage  = Js::StorageType::Memory;
    config.subjects = { "subject1", "subject2" }; // stream listen this subjects
    JsStreamPtr stream(js->getOrCreateStream(config), streamDeleter);

    // sync publishing, wait ack
    Js::PublishAck ack = js->publish(Message("subject", "data"));
    std::cout << ack.stream << ack.domain;

    // register async publishing error handler
    auto cb = [](Message msg, NatsMq::Status status, Js::Status error){
        // you can try send msg again or some what
    };

    js->registerAsyncErrorHandler(std::move(cb));

    // async publishing
    js->apublish(Message("async_subject", "async_data"));

    // you can wait until all msg has ack
    int waitTimoutMs = 5000;
    js->waitAsyncPublishCompleted(waitTimoutMs);

    // also you can return all messages that did not receive confirmation and process them somehow
    std::vector<Message> pendingMsgs = js->pendingMessages();
}  
 catch(const NatsMq::JsException& jsExc)
{
   std::cout << jsExc.what();
}
catch(const NatsMq::Exception& exc)
{
   std::cout << exc.what();
}
```

### Jet Stream subscribe
Subscribing to jetstream is also the same, but by default set manual ack and nack messages. 
Also a second type of PullSubscription is added. It differs in that you do not receive messages automatically, but must request them manually.

```
// just like first js example
try
{
      auto cb = [](Js::IncomingMessage msg) {
            std::cout << std::strig(msg) << std::endl;
            msg.ack(); // you must manual ack or nack msg
      };

      Js::Subscription sub = js->subscribe("stream_name", "subject", std::move(cb));

      // wait subscription data

      Js::PullSubscription pullSub = js->pullSubscribe("stream_name", "subject");

      int maxMsgCount = 5;
      int timeoutMs = 5000;
      std::vector<NatsMq::JsIncomingMessage> msgs = pullSub.fetch(maxMsgCount, timeoutMs);

      // handle fetched messages
}  
catch(const NatsMq::JsException& jsExc)
{
   std::cout << jsExc.what();
}
catch(const NatsMq::Exception& exc)
{
   std::cout << exc.what();
}
  
```

### Key Value store
Key-value store is an abstraction created with jetstream. Therefore, to access this store, we need an JetStream object.

#### Key value store managment
Managment operation allow you to request store name, all keys, create store, remove store. Store should not be used after destroy JetStream object.
```
// our agreements

try
{
     Js::KeyValue::Config config;
     config.bucket = "example_store";
     config.history = 10; // how many records of history will we keep, by default 1

     std::unique_ptr<Js::KeyValueStore> store(js->getOrCreateKeyValueStore(config));
     // we also use js->getKeyValueStore("storeName") but store may not exists

     std::vector<std::string> keys = store->keys();

     // KeyValueStore as a JsStream not remove the store on the server after destroy object
     store->deleteStore();
}  
catch(const NatsMq::JsException& jsExc)
{
   std::cout << jsExc.what();
}
catch(const NatsMq::Exception& exc)
{
   std::cout << exc.what();
}
```

#### Key value store put data
We will assume that the store already exists and stored in the variable store.
```
// our agreements
try
{
     // Set value if exists or create new key-value element
     store->put("my_key", "my_value");

     // Create new key-value element if not exists
     store->create("my_key1", "my_value1");
}  
catch(const NatsMq::JsException& jsExc)
{
   std::cout << jsExc.what();
}
catch(const NatsMq::Exception& exc)
{
   std::cout << exc.what();
}
```

#### Key value store get data
```
// our agreements
try
{
    // el it's info and value
    Js::KeyValue::Element el = store->get("key");
}  
catch(const NatsMq::JsException& jsExc)
{
   std::cout << jsExc.what();
}
catch(const NatsMq::Exception& exc)
{
   std::cout << exc.what();
}
```

#### Key value store remove data
```
// our agreements
try
{
    store->remove("key");
}  
catch(const NatsMq::JsException& jsExc)
{
   std::cout << jsExc.what();
}
catch(const NatsMq::Exception& exc)
{
   std::cout << exc.what();
}
```

### Object store
Object store is also an abstraction created by jetstream. So to access this storage we need a JetStream object. The object you are trying to put in the storage is divided into N parts of a certain size and each part is sent as a separate message.

#### Object store managment
```
// our agreements

try
{
     Js::ObjectStoreConfig config;
     config.bucket  = streamName;
     config.storage = Js::StorageType::Memory;
     config.maxAge  = 60; // max age in seconds. If 0 then u have an exception

     std::unique_ptr<Js::ObjectStore> store(js->getOrCreateObjectStore(config));
     // we also use js->getObjectStore("storeName") but store may not exists

     if(store->storeExists())
        std::cout << "We have object store! << std::endl;

     // ObjectStore as a JsStream not remove the store on the server after destroy object
     store->deleteStore();
}  
catch(const NatsMq::JsException& jsExc)
{
   std::cout << jsExc.what();
}
catch(const NatsMq::Exception& exc)
{
   std::cout << exc.what();
}
```

#### Object store remove data
```
// our agreements
try
{
    // remove object. if request object or request info after remove then meta.deleted == true
    os->remove(el.meta.name);
}  
catch(const NatsMq::JsException& jsExc)
{
   std::cout << jsExc.what();
}
catch(const NatsMq::Exception& exc)
{
   std::cout << exc.what();
}
```

#### Object store put data
```
// our agreements
try
{
     // Set value if exists or create new key-value element. Js::ObjectElement.value it's vector of uint8_t
     Js::ObjectElement element;
     element.meta.name = "name";

     std::string d("my large large data");
     el.data.insert(el.data.end(), d.begin(), d.end());

     store->put(element);
}  
catch(const NatsMq::JsException& jsExc)
{
   std::cout << jsExc.what();
}
catch(const NatsMq::Exception& exc)
{
   std::cout << exc.what();
}
```

#### Object store get data
```
// our agreements
try
{
    // get only object info
    const auto info = store->info("name");

    // get object info and data
    const auto object = store->get("name");
}  
catch(const NatsMq::JsException& jsExc)
{
   std::cout << jsExc.what();
}
catch(const NatsMq::Exception& exc)
{
   std::cout << exc.what();
}
```

### Advanced settings
Advanced connection setting is carried out by setting the settings through the function ```client->setOption()```. This function expects 2 arguments: the first is the value of the ```NatsMq::Option``` enum, the second is the value to be set.

**Note the "Expect type" column containing the exact type expected by the function. If you put int instead of int64_t you will get an std::bad_variant_access!**

Below is a table with default values ​​and a brief description. I'll leave links to the cnatc developer documentation if you need more information.

| Enum value              |Expect type            | Default value   | Description   |
|:-----------------------:|:---------------------:|:---------------:|:-------------:|
| Randomize               |bool                   | false           |If true, then provided list of server URLs is used in random order. [Link](http://nats-io.github.io/nats.c/group__opts_group.html#gaabf8763b931dcf389c83fd95d760a413)|
| Verbose                 |bool                   | false           |If true, then sends are echoed by the server with an OK protocol message. [Link](http://nats-io.github.io/nats.c/group__opts_group.html#gaa16bfd09ef318af18c27a0e199681b69)|
| Pedanic                 |bool                   | false           |If true, then some extra checks will be performed by the server. [Link](http://nats-io.github.io/nats.c/group__opts_group.html#ga26f166af20de98bec67bc6cc916f769e)|
| AllowRecconect          |bool                   | true            |Specifies whether or not the client library should try to reconnect when losing the connection to the NATS Server. [Link](http://nats-io.github.io/nats.c/group__opts_group.html#ga3d1d3cb2f2a0e23d27dd60e96d1cc91b)|
| RetryOnFailedConnect    |bool                   | true            |If true and connection can't be established right away, the library will attempt to connect based on the reconnect attempts and delay settings. [Link](http://nats-io.github.io/nats.c/group__opts_group.html#ga20946800d024b7089e73d63454d1c19f)|
| MaxReconnect            |int                    | 60              |Specifies the maximum number of reconnect attempts. [Link](http://nats-io.github.io/nats.c/group__opts_group.html#gab54cd2719c0b64eebd6c7b83dd2908a0)|
| SendAsap                |bool                   | false           |If true, then disables buffering and will make Publish calls send the data right away, reducing latency, but also throughput.[Link](http://nats-io.github.io/nats.c/group__opts_group.html#ga8f06568cc3319a5a0eef9f80282034ca)|
| DisableNoResponders     |bool                   | false           |If true, then disable "no responders" feature and all requests will wait until timeout. [Link](http://nats-io.github.io/nats.c/group__opts_group.html#ga192465163abb87ad48a843b45cdf9984)|
| UseGlobalMsgDelivery    |bool                   | true            |If true asynchronous subscribers will use a shared thread pool to deliver messages, if false then each subscriber has its own thread. [Link](http://nats-io.github.io/nats.c/group__opts_group.html#gabf060c92648b50c069f0abe7cbb06f1c)|
| FailRequestOnDisconnect |bool                   | false           |If true and disconnect occurred, then all request call ends with error. [Link](http://nats-io.github.io/nats.c/group__opts_group.html#gadef4376a5e608cbc8c1a9e2b6335dc79)|
| MaxPingsOut             |int                    | 2               |Maximum number of PINGs without corresponding PONGs (which should be received from the server) before closing the connection. [Link](http://nats-io.github.io/nats.c/group__opts_group.html#gaaff89c3f41627f4b2929dbc9b82d265b)|
| IOBufferSize            |int                    | 32768           |Size, in bytes, of the internal read/write buffers used for reading/writing data from a socket. [Link](http://nats-io.github.io/nats.c/group__opts_group.html#ga1a3e3abfd7ddd8aff247df8f332bbda3)|
| ReconnectBufferSize     |int                    | 8'388'608       |Size, in bytes, of the backing buffer holding published data while the library is reconnecting. [Link](http://nats-io.github.io/nats.c/group__opts_group.html#gaa0d4b7ece8477cb9879f0dafff3456a5)|
| MaxPendingMessages      |int                    | 65536           |Maximum number of inbound messages that can be buffered in the library, for each subscription, before inbound messages are dropped. [Link](http://nats-io.github.io/nats.c/group__opts_group.html#ga95510436eee06f9992ded96a44795c40)|
|        Timeout          |int64_t                | 2000            |This timeout, expressed in milliseconds, is used to interrupt a (re)connect attempt to a NATS Server. [Link](http://nats-io.github.io/nats.c/group__opts_group.html#ga17cd7fe41176cd98aca1184fa9352ad9)|
| PingInterval            |int64_t                | 120'000         |Interval, expressed in milliseconds, in which the client sends PING protocols to the NATS Server. [Link](http://nats-io.github.io/nats.c/group__opts_group.html#gae68fb615835364c0809555e8dc93f57e)|
| ReconnectWait           |int64_t                | 2000            |Interval, expressed in milliseconds, specifies how long to wait between two reconnect attempts from the same server. [Link](http://nats-io.github.io/nats.c/group__opts_group.html#gae68fb615835364c0809555e8dc93f57e)|
| Name                    |std::string            | **not defined** |This name is sent as part of the CONNECT protocol. There is no default name.[Link](http://nats-io.github.io/nats.c/group__opts_group.html#ga1c529d347be0fe2eec17c7f4698e283e)|
| Token                   |std::string            | **not defined** |To instruct the client library to use this token when connecting to a server that requires authentication. [Link](http://nats-io.github.io/nats.c/group__opts_group.html#gad58a5b9dabadeebda30e952ff7b39193)|
| UserCreds               |NatsMq::UserCredentials| **not defined** |To instruct the client library to use those credentials when connecting to a server that requires authentication. [Link](http://nats-io.github.io/nats.c/group__opts_group.html#ga5b99da7dd74aac3be962f323c3863d9e)|
