import ipaddress
import random
import secrets
import uuid
import logging
import sys


def is_address_valid(ip, port):
    try:
        ipaddress.ip_address(ip)
        port = int(port)
        if port >= 2 ** 16:
            return False

    except ValueError:
        return False

    return True


def int_to_byte_str(number: int, length: int) -> str:
    """
    Generate a string of specified length representing the binary presentation of the input number.

    :param number: Input number
    :param length: Length of the output string
    :return: string that represents the binary representation of the number
    """

    binary_str = format(number, 'b')
    if len(binary_str) > length:
        raise ValueError(f"Binary representation of {number} is longer than specified length {length}")
    return binary_str.zfill(length)


def binary_string_to_int(binary_string):
    try:
        decimal_number = int(binary_string, 2)
        return decimal_number
    except ValueError:
        print("Invalid binary string. Please provide a valid string of 0s and 1s.")


def get_random_port(start_range=1024, end_range=65535) -> int:
    return random.randint(start_range, end_range)


def gen_unique_id() -> str:
    return str(uuid.uuid4())


def gen_transaction_id() -> int:
    token = secrets.token_bytes(12)
    transaction_id_decimal = int.from_bytes(token, byteorder='big')
    return transaction_id_decimal


def string_to_byte_array(binary_string):
    if len(binary_string) % 8 != 0:
        raise ValueError("Binary string length must be a multiple of 8.")

    byte_array = bytearray()
    for i in range(0, len(binary_string), 8):
        byte_value = int(binary_string[i:i + 8], 2)
        byte_array.append(byte_value)

    return byte_array


class ColoredFormatter(logging.Formatter):
    COLORS = {
        'RESET': '\033[0m',
        'RED': '\033[91m',
        'YELLOW': '\033[93m',
        'BLUE': '\033[94m',
    }

    def format(self, record):
        log_message = super().format(record)
        if record.levelno == logging.INFO:
            return f"{self.COLORS['BLUE']}{log_message}{self.COLORS['RESET']}"
        elif record.levelno == logging.WARNING:
            return f"{self.COLORS['YELLOW']}{log_message}{self.COLORS['RESET']}"
        elif record.levelno == logging.ERROR:
            return f"{self.COLORS['RED']}{log_message}{self.COLORS['RESET']}"
        else:
            return log_message


def setup_custom_logger(name):
    formatter = ColoredFormatter(fmt='%(asctime)s [_%(levelname)s_]: %(message)s', datefmt='%Y-%m-%d %H:%M:%S')

    handler = logging.StreamHandler(sys.stdout)
    handler.setFormatter(formatter)

    logger = logging.getLogger(name)
    logger.setLevel(logging.DEBUG)
    logger.addHandler(handler)

    return logger


def log_indication_send(logger, addr, payload):
    logger.info(f"RECEIVE INDICATION       : from {addr[0]}:{addr[1]}"
                f"\n=======================================\n"
                f"{payload}"
                f"\n=======================================\n"
                )


def log_success_receive(logger, addr, payload):
    logger.info(f"RECEIVE SUCCESS RESPONSE :  from {addr[0]}:{addr[1]}"
                f"\n=======================================\n"
                f"{payload}"
                f"\n=======================================\n"
                )


def log_request_send():
    pass


def log_request_receive(logger, addr, payload):
    logger.info(f"RECEIVE REQUEST          : connection req from {addr[0]}:{addr[1]}"
                f"\n=======================================\n"
                f"connect to : {payload}"
                f"\n=======================================\n"
                )
