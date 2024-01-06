import logging
import socket
import sys
from abc import abstractmethod, ABC
from ipaddress import IPv4Address, ip_address
from queue import Queue

from typeguard import typechecked

# if it runs from other locations, then append other modules dir
try:
    from utils.utils import *
except ImportError:
    sys.path.append('..')
    from utils.utils import *

logging.basicConfig(level=logging.NOTSET)
logger = logging.getLogger('rsv')


class Receiver(ABC):
    @typechecked
    @abstractmethod
    def __init__(self, ip: str, port: int):
        self.ip: IPv4Address = ip_address(ip)
        self.port: int = port


class ReliableChannelReceiver(Receiver):

    def __init__(self, ip: str, port: int):
        super().__init__(ip, port)

        self.listen_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.listen_sock.bind((str(self.ip), self.port))

    def rcv_data(self, data_queue: Queue):
        while True:
            data, address = self.listen_sock.recvfrom(4096)
            logger.info("receiving : " + str(data))
            data_queue.put(data)


class AckWaitUdtReceiver(Receiver):

    def __init__(self, ip: str, port: int):
        super().__init__(ip, port)
        self.counter = 0

        self.listen_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.listen_sock.bind((str(self.ip), self.port))

    def rcv_data(self, data_queue: Queue):
        while True:
            data, address = self.listen_sock.recvfrom(4096)

            logger.info("receiving : " + data.decode("utf-8"))
            seq = (data.decode()).split('_')[0]
            payload = (data.decode())[len(seq) + 1:]
            print("the payload is : " + payload)
            if int(seq) == self.counter + 1:
                self.counter += 1
                data_queue.put(payload.encode())
            # send ack
            self.listen_sock.sendto(b"ack of " + data, address)


class PipelineReceiver(Receiver, ABC):
    @typechecked
    @abstractmethod
    def __init__(
            self, ip: str, port: int,
            pipeline_protocol: Pipeline_Protocol | int,
            window_size: int
    ):

        super().__init__(ip, port)

        if type(pipeline_protocol) is int:
            self.protocol: Pipeline_Protocol = Pipeline_Protocol(pipeline_protocol)
        else:
            self.protocol: Pipeline_Protocol = pipeline_protocol

        self.window_size = window_size


class GbnReceiver(PipelineReceiver):

    def __init__(
            self, ip: str, port: int,
            pipeline_protocol: Pipeline_Protocol | int,
            window_size: int
    ):
        super().__init__(ip, port, pipeline_protocol, window_size)
