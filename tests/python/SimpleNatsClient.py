import socket
from argparse import ArgumentParser
from typing import Callable
from sys import stdout


def create_argparser() -> ArgumentParser:
    parser = ArgumentParser(prog='NatsClient', description='Simple nats client without third party library')
    parser.add_argument('--cmd_port', default='8888', type=int, help='port for send command to calling process')
    parser.add_argument('--nats_port', default='4222', type=int, help='nats broker port, default=4222')
    parser.add_argument('--nats_address', default='127.0.0.1', help='nats broker address, default=127.0.0.1')
    parser.add_argument('--reply_subject', help='Subject to be answer when prompted')
    parser.add_argument('--reply_data', default='test', help='Reply to be answer when prompted')
    parser.add_argument('--publish_subject', help='Subject to publish by client')
    parser.add_argument('--publish_data', default='test', help='Data to publish by client')
    parser.add_argument('--subscribe_subject', help='Subject for client subscription')
    parser.add_argument('--subscribe_reply_subject', help='Subject for a client report on a subscribed message. '
                                                          'Default = arg(--subscribe_subject) + _reply')
    parser.add_argument('--subscribe_reply_data', default='ok',
                        help='Data for a client report on a subscription message')
    return parser


class CommandSender:
    def __init__(self, port: int) -> None:
        try:
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.connect(('127.0.0.1', port))
            self.connected = True
        except ConnectionRefusedError:
            self.connected = False

    def __del__(self):
        if self.connected:
            self.socket.close()

    def send(self, msg: str) -> None:
        if self.connected:
            self.socket.sendall(msg.encode())


class IncomingMessage:
    def __init__(self):
        self.subject = None
        self.inbox = None
        self.data = None
        self.len = None

    @staticmethod
    def _is_request_data(parts: list[str]) -> bool:
        return False if parts[3].isdigit() else True

    def is_request(self) -> bool:
        return bool(self.inbox)

    def parse(self, input_data: str) -> str:
        msg, data = input_data.split('\r\n', 1)
        parts = msg.split(' ')
        if parts[0] != 'MSG':
            return data

        self.subject = parts[1]
        if self._is_request_data(parts):
            self.len = int(parts[4])
            self.inbox = parts[3]
        else:
            self.len = int(parts[3])
        self.data = data[:self.len]
        return input_data[self.len:]


class Subscription:
    def __init__(self, sid: int, cb: Callable) -> None:
        self.sid = sid
        self.cb = cb


class SimpleNatsClient:
    def __init__(self, address: str, port: int) -> None:
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.connect((address, port))
        self.reply_subs = {}
        self.subscribe_subs = {}
        self.global_sid = 1

    def __del__(self) -> None:
        self.unsubscribe_all()
        self.socket.close()

    def unsubscribe_all(self) -> None:
        for _, subscription in self.subscribe_subs.items():
            self._unsubscribe_command(subscription.sid)
        for _, subscription in self.reply_subs.items():
            self._unsubscribe_command(subscription.sid)

    def subscribe(self, subject: str, cb: Callable) -> None:
        sid = self._get_sid_and_increment()
        self._subscribe_command(subject, sid)
        self.subscribe_subs[subject] = Subscription(sid, cb)

    def publish(self, subject: str, msg: str) -> None:
        self._publish_command(subject, msg)

    def register_reply(self, subject: str, cb: Callable) -> None:
        sid = self._get_sid_and_increment()
        self._subscribe_command(subject, sid)
        self.reply_subs[subject] = Subscription(sid, cb)

    def listen(self) -> None:
        print('Start listen...')
        data = ''
        while True:
            data += self.socket.recv(1024).decode()
            while "\r\n" in data:
                msg = IncomingMessage()
                data = msg.parse(data)
                if msg.is_request():
                    self._handle_request(msg)
                else:
                    self._handle_subscription(msg)

                if self._all_tasks_done():
                    return

    def _subscribe_command(self, subject: str, sid: int) -> None:
        sub = "SUB {} {}\r\n".format(subject, sid)
        self.socket.sendall(sub.encode())

    def _unsubscribe_command(self, sid: int) -> None:
        unsub = "UNSUB {}\r\n".format(sid)
        self.socket.sendall(unsub.encode())

    def _publish_command(self, subject: str, msg: str) -> None:
        pub = "PUB {} {}\r\n{}\r\n".format(subject, len(msg), msg)
        self.socket.sendall(pub.encode())

    def _handle_request(self, msg: IncomingMessage):
        if msg.subject in self.reply_subs:
            sub = self.reply_subs[msg.subject]
            self.publish(msg.inbox, sub.cb(msg.data))
            self._unsubscribe_command(sub.sid)
            del self.reply_subs[msg.subject]

    def _handle_subscription(self, msg: IncomingMessage):
        if msg.subject in self.subscribe_subs:
            sub = self.subscribe_subs[msg.subject]
            sub.cb(msg.data)
            self._unsubscribe_command(sub.sid)
            del self.subscribe_subs[msg.subject]

    def _all_tasks_done(self) -> bool:
        return len(self.subscribe_subs) == 0 and len(self.reply_subs) == 0

    def _get_sid_and_increment(self) -> int:
        sid = self.global_sid
        self.global_sid += 1
        return sid


def publish_if_arg(nats_client, arguments) -> None:
    if args.publish_subject:
        print(f'Add publish task: subject - {arguments.publish_subject}, msg - {arguments.publish_data}')
        nats_client.publish(arguments.publish_subject, arguments.publish_data)


def subscribe_if_arg(nats_client, arguments) -> None:
    if arguments.subscribe_subject:
        reply_subject = arguments.subscribe_reply_subject
        if reply_subject is None:
            reply_subject = arguments.subscribe_subject + '_reply'

        print(f'Add subscribe task: subject - {arguments.subscribe_subject}, '
              f'reply subject - {reply_subject}, reply - {arguments.subscribe_reply_data}')

        def subscribe_cb(msg):
            print(f'New message for subscribers: {msg}. Client reply: subject - {reply_subject}, '
                  f'reply - {arguments.subscribe_reply_data}')
            nats_client.publish(reply_subject, arguments.subscribe_reply_data)

        nats_client.subscribe(arguments.subscribe_subject, subscribe_cb)


def register_reply_if_arg(nats_client, arguments) -> None:
    if arguments.reply_subject:
        print(f'Add reply task: subject - {arguments.reply_subject}, reply - {arguments.reply_data}')

        def reply_cb(msg):
            print(f'New message for request: {msg}. Client reply: {arguments.reply_data}')
            return arguments.reply_data
        nats_client.register_reply(arguments.reply_subject, reply_cb)


def is_request_or_subscribe(arguments) -> bool:
    return bool(arguments.reply_subject is not None or arguments.subscribe_subject is not None)


if __name__ == '__main__':
    print('Start simple NATS client')
    arg_parser = create_argparser()
    args = arg_parser.parse_args()

    commander = CommandSender(args.cmd_port)
    client = SimpleNatsClient(args.nats_address, int(args.nats_port))

    publish_if_arg(client, args)
    subscribe_if_arg(client, args)
    register_reply_if_arg(client, args)

    print('Client ready', flush=True)
    commander.send('ready')

    if is_request_or_subscribe(args):
        client.listen()

    client.unsubscribe_all()
    commander.send('close')
    print('Close simple NATS client')
