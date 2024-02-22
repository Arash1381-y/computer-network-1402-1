"""
Client side of UDP HOLE PUNCHING
"""
import queue
import socket
import threading
from enum import Enum

from config import BUFFER_SIZE, SERVER_IP, SERVER_PORT, LOG_MODE
from shared.STUN_protocol import StunMessage, StunMessageType
from shared.utils import gen_unique_id, gen_transaction_id, setup_custom_logger, log_success_receive, \
    log_indication_send

logger = setup_custom_logger("Client")


class ClientConnectionState(Enum):
    NO_CONNECTION = 0,
    STUN_SERVER_WAIT = 1,
    PEER_CONNECTION_ESTABLISHED = 2,


class StunClient:

    def __init__(self, stun_server_ip: str, stun_server_port: int,
                 opt_ip: str | None = None,
                 opt_port: int | None = None,
                 opt_sid: int | None = None,
                 opt_did: int | None = None):
        """

        :param stun_server_ip:
        :param stun_server_port:
        """
        self.ss_ip: str = stun_server_ip
        self.ss_port: int = int(stun_server_port)
        self.sc_ip: str | None = opt_ip
        self.sc_port: int | None = opt_port
        self.dp_ip: str | None = None
        self.dp_port: int | None = None
        self.c_socket: socket.socket | None = None
        self.recv_buffer = queue.Queue()
        self.send_buffer = queue.Queue()
        self.c_id = opt_sid if opt_sid else gen_unique_id()
        self.dest_peer_id = opt_did if opt_sid else gen_unique_id()
        self.transactions = {}
        self.connection_state = ClientConnectionState.NO_CONNECTION

    def __gen_id_indication(self):
        """
        send its ID to STUN server. This will be used for
        mapping specific peers together
        """
        im = StunMessage()
        im.set_message_type(StunMessageType.INDICATION)
        # create a transaction id
        tid = gen_transaction_id()
        im.set_transaction_id(tid)
        self.transactions[tid] = StunMessageType.INDICATION
        # add client id as payload
        im.set_payload(str(self.c_id))
        return im.encode()

    def __gen_request_peer_addr(self):
        rm = StunMessage()
        rm.set_message_type(StunMessageType.REQUEST)
        # create a transaction id
        tid = gen_transaction_id()
        rm.set_transaction_id(tid)
        self.transactions[tid] = StunMessageType.REQUEST
        rm.set_payload(f"{self.dest_peer_id}")
        return rm.encode()

    def __create_socket(self):
        """
        create Internet socket for UDP connections

        note: same socket is being used for sending data and receiving data at the same
        time, therefore there does not exist 2 row in NAT and other peer can send and recv
        message directly.
        There is no need to bind client socket to any specific ip or port. This can be randomly
        chosen by system.
        """

        c_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        try:
            if self.sc_ip and self.sc_port:
                c_socket.bind((self.sc_ip, self.sc_port))
                if LOG_MODE:
                    logger.info(f"socket bind successfully: {self.sc_ip}:{self.sc_port}")
        except OSError:
            logger.error("Can Not Bind!")

        self.c_socket = c_socket

    def __send_thread(self):
        c_socket = self.c_socket

        # id register message
        im = self.__gen_id_indication()

        # request message
        rm = self.__gen_request_peer_addr()

        self.connection_state = ClientConnectionState.STUN_SERVER_WAIT

        if LOG_MODE:
            logger.info(f"SEND INDICATION   : Register to {self.ss_ip} : {self.ss_port}")

        c_socket.sendto(im, (self.ss_ip, self.ss_port))

        if LOG_MODE:
            logger.info(f"SEND REQUEST      : Request to {self.ss_ip} : {self.ss_port}")

        c_socket.sendto(rm, (self.ss_ip, self.ss_port))

        while True:
            if self.connection_state == ClientConnectionState.PEER_CONNECTION_ESTABLISHED:
                data = self.send_buffer.get().encode()
                c_socket.sendto(data, (self.dp_ip, self.dp_port))

    def __receive_thread(self):
        c_socket = self.c_socket

        while True:
            buffer, addr = c_socket.recvfrom(BUFFER_SIZE)

            if self.connection_state == ClientConnectionState.STUN_SERVER_WAIT:
                rm = StunMessage.decode(buffer)
                rm_id = rm.transaction_id
                rm_type = rm.message_type

                # check if request transaction is same as request
                if rm_id in self.transactions and self.transactions.get(rm_id) == StunMessageType.REQUEST:
                    if rm_type == StunMessageType.SUCCESS_RESPONSE:
                        self.dp_ip, self.dp_port = rm.extractPayload()
                        log_success_receive(logger, addr, rm.extractPayload())
                        self.connection_state = ClientConnectionState.PEER_CONNECTION_ESTABLISHED
                    elif rm_type == StunMessageType.ERROR_RESPONSE:
                        # TODO: handle error
                        pass

                # check if indication is received
                elif rm_type == StunMessageType.INDICATION:
                    if addr[0] == SERVER_IP and addr[1] == SERVER_PORT:
                        log_indication_send(logger, addr, rm.payload)

            elif self.connection_state == ClientConnectionState.PEER_CONNECTION_ESTABLISHED:
                print("<< " + buffer.decode())

    def run(self):
        self.__create_socket()

        event1 = threading.Event()
        event2 = threading.Event()
        event1.clear()
        event2.clear()

        sender = threading.Thread(target=self.__send_thread, )
        receiver = threading.Thread(target=self.__receive_thread, )

        sender.start()
        receiver.start()

        sender.join()
        receiver.join()

    def fill_buffer(self, data):
        # probably need to change this later
        self.send_buffer.put(data)

    def does_peer_connected(self):
        return self.connection_state == ClientConnectionState.PEER_CONNECTION_ESTABLISHED
