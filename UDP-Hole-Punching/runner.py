import argparse
import threading

from client.client import StunClient
from config import SERVER_IP, SERVER_PORT
from server.server import StunServer

parser = argparse.ArgumentParser()

parser.add_argument('-t', '--type', required=True, choices=["S", "C"], help="run as server or client")
parser.add_argument('-si', "--source-id", type=int, help="id that specifies this peer")
parser.add_argument('-di', "--dest-id", type=int, help="id that specifies the other peer")


def main():
    args = vars(parser.parse_args())

    runner_type = args['type']
    runner_source = args['source_id']
    runner_dest = args['dest_id']

    if runner_type == "S":
        s = StunServer(SERVER_IP, SERVER_PORT)
        s.run()

    elif runner_type == "C" and runner_dest is not None:
        c = StunClient(SERVER_IP, SERVER_PORT,
                       opt_sid=runner_source,
                       opt_did=runner_dest)

        client_agent = threading.Thread(target=c.run)
        client_agent.start()

        while True:
            if c.does_peer_connected():
                usr_input = input()
                print(">> " + usr_input)
                c.fill_buffer(usr_input)


if __name__ == '__main__':
    main()
