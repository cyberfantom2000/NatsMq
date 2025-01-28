#include <NatsMq>
#include <sstream>

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>

int main(int argc, char** argv)
{
    try
    {
        std::unique_ptr<NatsMq::Client> client(NatsMq::Client::create());
        client->connect({ "nats://172.20.73.29:4222" });

        std::unique_ptr<NatsMq::JetStream> js(client->jetstream());

        qDebug() << "Exists streams:";
        for (auto&& el : js->streamNames())
            qDebug() << el.c_str();

        const auto infos = js->streamsInfo();

        qDebug() << "----------------------------------------------";

        for (auto&& info : infos)
        {

            qDebug() << info.config.name.c_str() << ". Subjects:";
            for (auto&& s : info.config.subjects)
                qDebug() << s.c_str();

            qDebug() << "Messages:" << info.state.messages;
            qDebug() << "Bytes:" << info.state.bytes;

            for (auto&& st : info.state.subjects)
                qDebug() << "Subject: " << st.subject.c_str() << "messages:" << st.messages;

            qDebug() << "----------------------------------------------";
        }

        //        std::unique_ptr<NatsMq::Js::Subscription> sub(js->subscribe("$O.test.M.t", "OBJ_test", [](NatsMq::Js::IncomingMessage msg) {
        //            qDebug() << "Incoming message!" << msg.msg.data;
        //            msg.ack();
        //        }));

        //        js->publish(NatsMq::Message("$O.test.M.t", "hello"));

        //        std::unique_ptr<NatsMq::Js::ObjectStore> os(js->getObjectStore("test"));

        //        const auto obj = os->get("firmware1");

        //        qDebug() << obj.meta.name.c_str() << obj.meta.size << obj.meta.chunks;
    }
    catch (const NatsMq::JsException& exc)
    {
        qDebug() << "JS error occured:" << exc.what();
    }
    catch (const NatsMq::Exception& exc)
    {
        qDebug() << "Error occured:" << exc.what();
    }
    return 1;
}
