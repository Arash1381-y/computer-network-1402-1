import queue
import threading

import sys

try:
    from config import CLIENT_IP, CLIENT_PORT, UDT_LISTEN_IP, UDT_LISTEN_PORT
    from dt.sender import ReliableChannelSender, AckWaitUdtSender
except ImportError:
    sys.path.append('..')
    from config import CLIENT_IP, CLIENT_PORT, UDT_LISTEN_IP, UDT_LISTEN_PORT
    from dt.sender import ReliableChannelSender, AckWaitUdtSender

from dt.receiver import ReliableChannelReceiver


class Client:

    def __init__(self, listen_to_ip: str, listen_to_port: int, rdt_to_ip: str, rdt_to_port: int):
        self.listen_to_ip = listen_to_ip
        self.listen_to_port = listen_to_port

        self.rdt_to_ip = rdt_to_ip
        self.rdt_to_port = rdt_to_port
        self.buffer = queue.Queue()
        self.receiver = ReliableChannelReceiver(listen_to_ip, listen_to_port)
        self.sender = AckWaitUdtSender(rdt_to_ip, rdt_to_port)

    def __send_data(self):
        # for now it just send data when buffer is not empy
        self.sender.snd_data(self.buffer)

    def __recv_data(self):
        # first we just need to listen to ip and port and fill the buffer
        self.receiver.rcv_data(self.buffer)

    def run(self):
        # one thread send data
        receiver = threading.Thread(target=self.__recv_data)
        # one thread recv data
        sender = threading.Thread(target=self.__send_data)

        receiver.start()
        sender.start()

        receiver.join()
        sender.join()


if __name__ == '__main__':
    client = Client(
        listen_to_ip=CLIENT_IP,
        listen_to_port=CLIENT_PORT,
        rdt_to_ip=UDT_LISTEN_IP,
        rdt_to_port=UDT_LISTEN_PORT
    )

    client.run()
