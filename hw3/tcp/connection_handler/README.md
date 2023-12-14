# connection_handler

## Info

Handling proxy server traffic using TCP connections. The `handle_connection` is the main function of this header.

## handle_function() - connection_handler.h

```c
int handle_connection(
        int passive_sfd,
        struct sockaddr_in server_add,
        logger_t *l
        );
```

### Description

Call by server to handle a connection. The handler proxy the traffic to packet destination.
The supported requests includes http and https requests.

`passive_sfd` socked file descriptor which listens on `server_add` addr and port.

`server_add` struct containing port and address of server listening socket

l logger

return -1 if error otherwise 0

#### code

```c

struct sockaddr_in client_addr;
socklen_t lclient_addr = sizeof(client_addr);
int active_cli_srv_sfd = accept(passive_sfd, (struct sockaddr *) &client_addr, &lclient_addr);
```

create an `addr_in` struct for retrieving the client information. size of `client_addr` is needed for matching the
`sockaddr_in` to generic `sockaddr` struct. The `accept` function returns a new active socket file descriptor for
(client_addr, server_addr) pair.

```c
  // get first message from client
    char buff[BUFF_SIZE];
    ssize_t ok = recv(active_cli_srv_sfd, buff, BUFF_SIZE, 0);
    if (ok < 0) {
        error(l, "error reading from socket");
        return -1;
    }
```

Read the first message from client. The message is the first packet sent by client. The packet contains the
destination address and port which is needed to create a new socket for proxying the traffic.



