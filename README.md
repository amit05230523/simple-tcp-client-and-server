
File "simple_tcp_client.c" implements a simple tcp client and file
"simple_tcp_server.c" implements a simple tcp server.

Both programs take the same two arguments: server's IPv4 address and the
server's port number on which the server will/is listen/listening for new
connections.

The user can connect the server and the client and then send messages to each
other.

Although the tcp server runs in an infinite loop, it accepts only one connection
at a time and communicates with that client only at a time. Only when this
client closes the connection, then the tcp server will accept another
connection.

However, when you try to connect the second client to the server, the
connection will succeed but you won't be able to send/recv messages to/from the
server. The second client will connect to the server because the linux kernel
will complete TCP's 3-way handshake between the client and the server and then
put this connection in the server's backlog queue for the sever to accept. But
this tcp server won't accept any new connection until the current client has
closed the connection, so the second client won't be able to communicate with
the server. Same will be the case for all other clients.

---- End of README ----
