import socket
from typing import TypeVar
from config import BUFFER_SIZE, LOG_MODE
from shared.STUN_protocol import StunMessage, StunMessageType
from shared.utils import setup_custom_logger, gen_transaction_id, log_success_receive, log_request_receive, \
    is_address_valid
from prettytable import PrettyTable

PeerPublicInfo = TypeVar('PeerPublicInfo')
PeerID = TypeVar('PeerID')

logger = setup_custom_logger("Server")


class MatchTable:
    def __init__(self):
        self.info_id_dict: dict[PeerPublicInfo, PeerID] = {}
        self.id_info_dict: dict[PeerID, PeerPublicInfo] = {}

    def add_peer(self, pid: PeerID, p_ip, p_port):
        pidt: PeerPublicInfo = str(p_ip) + ":" + str(p_port)
        # TODO: handle duplicate user id -> send error Response
        self.info_id_dict[pidt] = pid
        self.id_info_dict[pid] = pidt

    def get_pid(self, p_ip, p_port):
        pidt: PeerPublicInfo = str(p_ip) + ":" + str(p_port)
        if pidt in self.info_id_dict:
            return self.info_id_dict[pidt]
        else:
            return None

    def get_pinfo(self, pid):
        if pid in self.id_info_dict:
            ip, port = self.id_info_dict[pid].split(":")
            return ip, int(port)
        else:
            return None

    def remove_pid(self, pid: PeerID, p_ip, p_port):
        pass

    def log_table(self):
        t = PrettyTable(['Public IP:Port', 'Peer ID'])
        for (key, val) in self.info_id_dict.items():
            t.add_row([key, val])

        return t.get_string()


class StunServer:

    def __init__(self, stun_server_ip: str, stun_server_port: int):
        self.ss_ip = stun_server_ip
        self.ss_port = stun_server_port
        self.match_table = MatchTable()
        self.s_socket: socket.socket | None = None
        self.pending_requests: dict[PeerID, (int, PeerID)] = {}

    def __create_socket(self):
        s_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s_socket.bind((self.ss_ip, int(self.ss_port)))
        self.s_socket = s_socket

    def __handle_register(self, addr, sm: StunMessage):
        ip, port = addr
        if not is_address_valid(ip, port):
            return None

        pid = sm.payload
        self.match_table.add_peer(pid, ip, port)
        logger.info(f"LOG: REGISTER ID {pid}\n" + self.match_table.log_table())
        return pid

    def __handle_request(self, addr1, sm: StunMessage):
        # check if address is valid
        p1_ip, p1_port = addr1
        if not is_address_valid(p1_ip, p1_port):
            return None

        # check if ip<->port is registered
        pid1 = self.match_table.get_pid(p1_ip, p1_port)
        if not pid1:
            logger.error("RECEIVED REQUEST: Not register peer")
            return None

        # add request to pendings
        pid2 = sm.payload
        self.pending_requests[pid1] = (sm.transaction_id, pid2)

        addr2 = self.match_table.get_pinfo(pid2)
        if not addr2:
            logger.error("RECEIVED REQUEST: Unknown dest peer -> pending req...")
            return None

        p2_ip, p2_port = addr2
        req1_2 = self.pending_requests.pop(pid1)
        req2_1 = self.pending_requests.pop(pid2)

        if req1_2[1] == pid2 and req2_1[1] == pid1:
            return (req1_2[0], (p2_ip, int(p2_port))), (req2_1[0], (p1_ip, int(p1_port))),

        return None

    def __gen_publicInfo_indication(self, addr):
        im = StunMessage()
        im.set_message_type(StunMessageType.INDICATION)
        transaction_id = gen_transaction_id()
        im.set_transaction_id(transaction_id)
        im.set_payload(f"PUBLIC INFO: {addr}")
        return im.encode()

    def __gen_success_response(self, transaction_id, public_info):
        sm = StunMessage()
        sm.set_transaction_id(transaction_id)
        sm.set_payload(f"{public_info[0]}:{public_info[1]}")
        sm.set_message_type(StunMessageType.SUCCESS_RESPONSE)
        return sm.encode()

    def run(self):
        self.__create_socket()

        s_socket = self.s_socket
        while True:
            buffer, addr = s_socket.recvfrom(BUFFER_SIZE)
            sm: StunMessage = StunMessage.decode(buffer)

            # check class
            if sm.message_type == StunMessageType.INDICATION:

                if LOG_MODE:
                    log_success_receive(logger, addr, sm.payload)

                if self.__handle_register(addr, sm):
                    im = self.__gen_publicInfo_indication(addr)
                    s_socket.sendto(im, addr)

            if sm.message_type == StunMessageType.REQUEST:
                if LOG_MODE:
                    log_request_receive(logger, addr, sm.payload)

                addresses = self.__handle_request(addr, sm)
                if addresses:
                    sm1 = self.__gen_success_response(*addresses[0])
                    sm2 = self.__gen_success_response(*addresses[1])
                    s_socket.sendto(sm1, addresses[1][1])
                    s_socket.sendto(sm2, addresses[0][1])
                    logger.info(
                        f"Connect Peer {addresses[1][1]} to Peer {addresses[0][1]}")
