cmake_minimum_required(VERSION 3.27)
project(tcp C)

set(CMAKE_C_STANDARD 11)


set(CMAKE_C_FLAGS "-fopenmp")

add_executable(tcp main.c
        http_parser/http_parser.c
        socket/tcp_socket.c
        socket/tcp_socket.h
        data_types/http_request.h
        http_parser/http_parser.h
        connection_handler/connection_handler.c
        connection_handler/connection_handler.h
        data_types/http_request.c
        CbNB/CbNB.c
        CbNB/CbNB.h
        argparse/argparse.c
)


include_directories("./CbNB" "./http_parser" "./socket" "./data_types" "./connection_handler" "./argparse")