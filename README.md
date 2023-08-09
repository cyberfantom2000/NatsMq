# NatsMq

Cpp wrapper over the [official cnats library](https://github.com/nats-io/nats.c).

- [Installing](#installing)
- [Tests](#tests)
- [Examples](#examples)
    * [Create core client](#create-core-client)
    * [Publish](#publish)
    * [Subscribe](#subscribe)
    * [Request](#request)
    * [Reply](#reply)
    * [Create JetStream client](#create-jetstream-client)
    * [Jet Stream publish](#jet-stream-publish)
    * [Jet Stream subscribe](#jet-stream-subscribe)
    * [Key Value store](#key-value-store)
        * [Key value store managment](#key-value-store-managment)
        * [Key value store set data](#key-value-store-set-data)
        * [Key value store get data](#key-value-store-get-data)
        * [Key value store remove data](#key-value-store-get-data)
- [Communication](#communication)

## Installing
You can install library in your project use Fetch Content:
```
include(FetchContent)

FetchContent_Declare(
  natsmq
  GIT_REPOSITORY https://github.com/cyberfantom2000/NatsMq.git
  GIT_SHALLOW    ON
)

set(NATSMQ_BUILD_TESTS OFF)
set(NATS_BUILD_EXAMPLES OFF)

FetchContent_MakeAvailable(natsmq)

target_link_libraries(${PROJECT_NAME} NatsMq)
```
You can also add the project as a submodule or by any other means.
On windows has a dependency on the winsock32 library. For build test required Qt5::Core and Qt5::Network.

## Tests
To run the tests you will need to install server nats and python3. I recomend using [docker image](https://hub.docker.com/_/nats) for server. [Python3](https://www.python.org/downloads/) used for the second command line nats client and use only standart python libraries. Also test project used [GTEST framework](https://github.com/google/googletest). But this will be picked up automatically with cmake FetchContent.
Start the NATS server first, then run a tests. For a JetStream tests, the server must be started with the flag ```--jetstream```.

## Examples
Please note, there may be typos or errors, for specific examples, look in the examples folder.

### Create core client
For create client you must use static function ```NatsMq::Client::configureAndCreate(int)```. This function return pointer on a client and you must take care about of freeing the memory. This function expect thread pool size as a parameter, by default 1. This parameter depends on the load and performance of your machine. 
Most functions throw exceptions when an error occurs, so you must handle them. All exceptions inherited from std::runtime_error, they have a "what()" method and a public "status" field that stores error code. 
when the client is destroyed, it will close the connection to the server.

```
#include <NatsMq>
#include <memory>
#include <iostream>

int main()
{
  std::unique_ptr<NatsMq::Client> client(NatsMq::Client::configureAndCreate());
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
Calling ```subscribe``` method on client you will get Subscription object. Using this object you can register message handler and get subscription statistics.Please note that when the object is destroyed, an unsubscribe from the topic will occur.

Method ```registerListener``` expect callback with ```void(*)(const NatsMq::IncomingMessage&)``` signathure. The passed callback will be moved to object if is rvalue and copy and move if is lvalue.
```
try
{
   NatsMq::Subscription sub = client->subscribe("interesting_subject");
   sub.registerListener([](const NatsMq::IncomingMessage& msg){
       std::cout << "Subject: " << msg.subject << "; Data: " << msg.data.constData()  << std::endl;
   });

   // Wait subscription data

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
using FutureMsg = std::future<NatsMq::IncomingMessage>;

try
{
    auto printMsg = [](auto&& msg){
      std::cout << "Subject: " << msg.subject
                << "; Data: " << msg.data.constData()  << std::endl;                
    };               

    int timeoutMs = 2000;
    NatsMq::IncomingMessage msg = client->request(Message("req_subject", "req_data"), timeoutMs);
    printMsg(msg);

    FutureMsg futureMsg = client->asyncReuest(Message("req_subject", "req_data"), timeoutMs);
    msg = futureMsg.get(); // wait std::future result or exception
    printMsg(msg);
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


int main()
{
  
  std::unique_ptr<NatsMq::Client> client(NatsMq::Client::configureAndCreate());
  try
  {
      NatsMq::Options options;
      options.sendAsap = true;
      client->connect({"nats://localhost:4222"}, options);

      auto replyCb = [&client](NatsMq::IncomingMessage msg) {
            /* handle incoming msg */
            std::cout << msg.data.constData();

            /* make reply */
            msg.data = "reply_data";
            try
            {
                client->publish(msg);
            }
            catch (const NatsMq::Exception& exc)
            {
                std::cout << exc.what();
            }
        };

      auto sub = client->subscribe("reply_subject");
      sub.registerListener(std::move(replyCb));

      // wait request
  }
  catch(const NatsMq::Exception& exc)
  {
      std::cout << exc.what();
  }
  return 0;
}
```

### Create JetStream client
For create JetStream client you must first create and connect core client. After you get the JetStream from the core client, it can be removed. This will not close the connection because it is shared between them.
Before publishing, you need to make sure that a stream listening to your subjects exists. Otherwise you will get an exception. You may used 2 function to get or create stream: ```getStream``` or ```getOrCreateStream```.

```
#include <NatsMq>
#include <iostream>
#include <memory>

namespace
{
    // JsStream not remove the stream on the server
    auto streamDeleter = [](NatsMq::JsStream* stream) {stream->remove(); delete stream; };
    using JsStreamPtr  = std::unique_ptr<NatsMq::JsStream, decltype(streamDeleter)>;
}

int main()
{
    std::unique_ptr<NatsMq::Client> client(NatsMq::Client::configureAndCreate());
    
    try
    {
        client->connect({ "nats://localhost:4222" });
        std::unique_ptr<NatsMq::JetStream> js(client->createJetStream());

        NatsMq::JsStreamConfig config;
        config.name     = "my_stream";
        config.storage  = NatsMq::JsStreamConfig::Storage::Memory;
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
    NatsMq::JsStreamConfig config;
    config.name     = "my_stream";
    config.storage  = NatsMq::JsStreamConfig::Storage::Memory;
    config.subjects = { "subject1", "subject2" }; // stream listen this subjects
    JsStreamPtr stream(js->getOrCreateStream(config), streamDeleter);

    // sync publishing, wait ack
    NatsMq::JsPublishAck ack = js->publish(Message("subject", "data"));
    std::cout << ack.stream << ack.domain;

    // async publishing
    auto cb = [](const NatsMq::Message& msg, NatsMq::Status status, NatsMq::JsError error){
        // you can try send msg again or some what
    };

    js->registerAsyncErrorHandler(std::move(cb));

    ack = js->asyncPublish(Message("async_subject", "async_data"));

    // you can wait until all msg has ack
    int waitTimoutMs = 5000;
    js->waitAsyncPublishCompleted(waitTimoutMs);

    // also you can return all messages that did not receive confirmation and process them somehow
    std::vector<NatsMq::Message> pendingMsgs = js->getAsyncPendingMessages();
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
Subscribe method expect 2 requires and 1 optional arguments: stream name, subject, consumer.

```
// just like first js example
try
{
      NatsMq::JsSubscription sub = js->subscribe("stream_name", "subject");

      sub.registerListener([](const NatsMq::JsIncomingMessage& msg) {
            std::cout << msg.data.constData();
            msg.ack(); // you must manual ack or nack msg
      });

      // wait subscription data

      NatsMq::JsPullSubscription pullSub = js->pullSubscribe("stream_name", "subject");

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
In order not to copy the code, let's agree that in all the examples I have already done the following:

```
 std::unique_ptr<NatsMq::Client> client(NatsMq::Client::configureAndCreate());
 client->connect({ "nats://localhost:4222" });

 std::unique_ptr<NatsMq::JetStream> js(client->createJetStream());

 NatsMq::JsStreamConfig config;
 config.name     = "my_stream";
 config.storage  = NatsMq::JsStreamConfig::Storage::Memory;
 config.subjects = { "subject1", "subject2" }; // stream listen this subjects

 JsStreamPtr stream1(js->getOrCreateStream(config), streamDeleter);
```

#### Key value store managment
Managment operation allow you to request store name, all keys, create store, remove store. Store should not be used after destroy JetStream object.
```
// our agreements

try
{
     NatsMq::KeyValueConfig config;
     config.name = "my_store";
     config.history = 10; // how many records of history will we keep, by default 1

     std::unique_ptr<NatsMq::KeyValueStore> store(js->getOrCreateStore(config));
     // we also use js->get("storeName") but store may not exists

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

#### Key value store set data
We will assume that the store already exists and stored in the variable store.
```
// our agreements
try
{
     // Set value if exists or create new key-value element
     store->putElement("my_key", "my_value");

     // Create new key-value element if not exists
     store->createElement("my_key1", "my_value1");
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
     NatsMq::ByteArray data = js->getElement("key");
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
    js->removeElement("key");
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

## Communication
You can contact me about the problem and improvements by mail Tak.sebek@yandex.ru. Please note that I can take a long time to answer.
  
