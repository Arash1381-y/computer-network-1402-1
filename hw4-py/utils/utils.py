from enum import Enum


class Pipeline_Protocol(Enum):
    GBN = 1,  # go-back-N protocol
    SR = 2  # selective-repeat protocol
