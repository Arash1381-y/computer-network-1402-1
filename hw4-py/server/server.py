import queue
import sys
import threading

try:
    from config import SERVER_IP, SERVER_PORT, UDT_FORWARD_IP, UDT_FORWARD_PORT
    from dt.sender import ReliableChannelSender
except ImportError:
    sys.path.append('..')
    from config import SERVER_IP, SERVER_PORT, UDT_FORWARD_IP, UDT_FORWARD_PORT
    from dt.sender import ReliableChannelSender

from dt.receiver import AckWaitUdtReceiver


class Server:

    def __init__(self, rdt_listen_to_ip: str, rdt_listen_to_port: int, forward_to_ip: str, forward_to_port: int):
        self.rdt_listen_to_ip = rdt_listen_to_ip
        self.rdt_listen_to_port = rdt_listen_to_port

        self.forward_to_ip = forward_to_ip
        self.forward_to_port = forward_to_port
        self.buffer = queue.Queue()
        self.receiver = AckWaitUdtReceiver(rdt_listen_to_ip, rdt_listen_to_port)
        self.sender = ReliableChannelSender(forward_to_ip, forward_to_port)

    def __send_data(self):
        # TODO: make it rdt
        # for now it just send data when buffer is not empy
        self.sender.snd_data(self.buffer)

    def __recv_data(self):
        # first we just need to listen to ip and port and fill the buffer
        self.receiver.rcv_data(self.buffer)

    def run(self):
        # one thread send data
        threading.Thread(target=self.__recv_data).start()
        # one thread recv data
        threading.Thread(target=self.__send_data).start()


if __name__ == '__main__':
    client = Server(
        rdt_listen_to_ip=UDT_FORWARD_IP,
        rdt_listen_to_port=UDT_FORWARD_PORT,
        forward_to_ip=SERVER_IP,
        forward_to_port=SERVER_PORT
    )

    client.run()
