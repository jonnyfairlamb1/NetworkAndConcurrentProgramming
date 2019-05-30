# TCP/UDP echo server and SFML client in C++

## Server

To run the server:

```bash
cd server
mix compile
mix run --no-halt
```

The code for the server is in `server/lib/server.ex`

## Client

To run the client:

```bash
make
./sfml-net
```

If you are planning to compile this in some other environment, make
sure that you link against the right SFML libraries (there is the
default location of this for the Linux labs in the first line of
the Makefile).

The client can be run on a different machine as long as the ip address
is correct for TCP connections and for UDP sending.

The client should connect to the server and send a message with TCP,
send a message with UDP, receive a message with TCP and receive a
message with UDP. Spend some time looking at the file for the C++ client
which is in `sfml-net.cpp` to familiarise yourself with SFML sockets.

For reference, there is the equivalent client coded in Elixir in the
file `client.ex`. To run the Elixir client:

```bash
iex client.ex
iex> Client.start # in the shell
```

Once more you should first make sure that the IP address/port numbers
match that of the server.

On Unix, we can find the IP address from the command line with
`ifconfig`, on Windows, it is `ipconfig`.
