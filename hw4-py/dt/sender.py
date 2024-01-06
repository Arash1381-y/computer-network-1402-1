import errno
from asyncio import sleep
from queue import Queue
import sys
import threading
import logging
from abc import abstractmethod, ABC
from ipaddress import IPv4Address, ip_address
from typing import List

from typeguard import typechecked

import socket
from socket import socket, AF_INET, SOCK_DGRAM  # import

# if it runs from other locations, then append other modules dir
try:
    from utils.utils import *
except ImportError:
    sys.path.append('..')
    from utils.utils import *

from config import WINDOW_SIZE

logging.basicConfig(level=logging.NOTSET)
logger = logging.getLogger('snd')


class Sender(ABC):

    @typechecked
    @abstractmethod
    def __init__(self, ip: str, port: int):
        self.ip: IPv4Address = ip_address(ip)
        self.port: int = port
        self.sock = socket(AF_INET, SOCK_DGRAM)
        self.sock.setblocking(False)

    def disband(self):
        sock = self.sock
        sock.shutdown(socket.SHUT_RDWR)
        sock.close()


class ReliableChannelSender(Sender):

    def __init__(self, ip: str, port: int):
        super().__init__(ip, port)

    def snd_data(self, data_queue: Queue):
        while True:
            if not data_queue.empty():
                data = data_queue.get()
                logger.info("sending : " + data.decode())
                self.sock.sendto(data, (str(self.ip), self.port))


class AckWaitUdtSender(Sender):
    def __init__(self, ip: str, port: int):
        super().__init__(ip, port)
        self.counter = 1

    def __add_seq(self, data):
        return f"{self.counter}_{data.decode()}".encode()

    def __wait_for_ack(self, event: threading.Event):
        while True:
            try:
                response, _ = self.sock.recvfrom(4096)
                logger.info("the response is " + str(response))
                self.counter += 1
                event.set()
            except:
                continue

    def __send_if_ack(self, queue: Queue, event: threading.Event):
        while True:
            if not queue.empty():
                event.clear()
                data = self.__add_seq(queue.get())

                logger.info("sending : " + str(data))
                self.sock.sendto(data, (str(self.ip), self.port))
                while not event.wait(.2):
                    self.sock.sendto(data, (str(self.ip), self.port))
                    self.send_next = False

    def snd_data(self, data_queue: Queue):
        event = threading.Event()
        waiter = threading.Thread(target=self.__wait_for_ack, args=(event,))
        sender = threading.Thread(target=self.__send_if_ack, args=(data_queue, event))

        waiter.start()
        sender.start()

        waiter.join()
        sender.join()


class PipelineSender(Sender):

    @typechecked
    @abstractmethod
    def __init__(
            self, udt_listen_ip: str, udt_listen_port: int,
            pipeline_protocol: Pipeline_Protocol, window_size: int
    ):
        super().__init__(udt_listen_ip, udt_listen_port)
        self.pipline_protocol = pipeline_protocol
        self.window_size = window_size


class GbnSender(PipelineSender):

    @typechecked
    def __init__(
            self, udt_listen_ip: str, udt_listen_port: int, window_size: int
    ):
        super().__init__(udt_listen_ip, udt_listen_port, Pipeline_Protocol.GBN, window_size)
        self.base = 1
        self.next_seq = 1
        self.timeout = False

    def __window_acked(self) -> bool:
        return self.base == self.next_seq

    def __send(self):
        pass

    def __ack(self):
        pass

    def run_sender(self):
        # create a thread for sending
        t_send = threading.Thread(target=None)
        t_ack = threading.Thread(target=None)
        # create a thread for updating seq
