from enum import Enum
from shared.utils import int_to_byte_str, binary_string_to_int, gen_transaction_id, string_to_byte_array

# TODO: add code for errors
INVALID_TYPE = ValueError("{}: Invalid Message Type")


class StunMessageType(Enum):
    REQUEST = "00000000000000"
    SUCCESS_RESPONSE = "00000100000000"
    ERROR_RESPONSE = "00000100010000"
    INDICATION = "00000000010000"

    def to_string(self):
        return self.value

    @staticmethod
    def from_string(s: str):
        try:
            return StunMessageType(s)
        except ValueError:
            raise ValueError("INVALID MESSAGE TYPE")


class StunMessage:
    def __init__(self):
        self.transaction_id: int | None = None
        self.message_type: StunMessageType | None = None
        self.message_len: int = 0
        self.magic_cookie = 0x2112A442
        self.transaction_id: int | None = None
        self.payload = ""

    def set_transaction_id(self, tid):
        self.transaction_id = tid

    def set_message_type(self, m_type: StunMessageType):
        self.message_type = m_type

    def set_payload(self, payload):
        self.payload = payload
        self.message_len = len(payload)

    def encode(self):
        e_message = ""
        # ================ HEADER ==================================
        # append 2 zero bit
        e_message += "00"
        # append message type
        e_message += self.message_type.to_string()
        # append message len
        e_message += int_to_byte_str(self.message_len, length=16)
        # append magic cookie
        e_message += int_to_byte_str(self.magic_cookie, length=32)
        # append transaction id
        e_message += int_to_byte_str(self.transaction_id, length=96)
        # convert to bytes
        be_message = string_to_byte_array(e_message)
        # ================ PAYLOAD ==================================
        be_message += self.payload.encode()
        return be_message

    @staticmethod
    def decode(b: bytes):
        header = b[:20]
        binary_strings = ''.join([bin(byte)[2:].zfill(8) for byte in header])
        message_class = StunMessageType.from_string(binary_strings[2:16])
        message_len = binary_string_to_int(binary_strings[16:32])
        magic_cookie = binary_string_to_int(binary_strings[32:64])
        transaction_id = binary_string_to_int(binary_strings[64:])
        payload = b[20:].decode()
        # TODO: handling large payloads by using message len

        dm = StunMessage()
        dm.set_message_type(message_class)
        dm.set_transaction_id(transaction_id)
        dm.set_payload(payload)

        # TODO: check magic cookie
        return dm

    def extractPayload(self):
        IP, port = self.payload.split(":")
        return IP, int(port)


im = StunMessage()
im.set_message_type(StunMessageType.INDICATION)
# create a transaction id
tid = gen_transaction_id()
im.set_transaction_id(tid)
im.set_payload("hello world")
test = im.encode()
btest = (bytes(test))
bdecode = StunMessage.decode(btest)
