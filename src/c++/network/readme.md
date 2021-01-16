# Networking

## Network Traffic Description

Server & Client communicate via two TCP network sockets.

1. `Control Packets` (client -> server): Literally controls the robot and tells it what to do based on input from web app
    * See [`ctrl_pkt_samples.json`](ctrl_pkt_samples.json) to inspect what the packet looks like
2. `Server Packets` (server -> client): Contains data obtained from the robot (i.e. camera frames) that client needs to relay to web app
    * See [`server_pkt_samples.json`](./server_pkt_samples.json) to inspect what the packet looks like

## Protocols

The protocol for sending & receiving packets is defined within the tcp_base.h/cpp files.
Pay special attention to the `sendData()` & `recvData()` functions which handles the inevitable packet partitioning of packets that occurs when sending and receiving packets of variable sizes.

All network packets are serialized into jsons (and then bsons, aka binary jsons) using the [`nlohmann::json library`](https://github.com/nlohmann/json) to prevent the need for unrobust & tedious effort of manually bit packing. Additionally, transferring them as jsons enables the client to effortless exchange data with the backend/frontend without needing to convert too and from structs constantly (especially when dealing with the js side of the frontend). The creation, parsing, serialization, and deserialization of packets can all be found within the `packet.h/cpp` files.

## Class Heirarchy

Packet -> TcpBase -> TcpServer/TcpClient

## Running

* At runtime, main creates a TcpBase object casted from **either** a client or server object depending on the situation.
* TcpBase starts up two threads when `runNetAgent()` is called to exchange both control & server data packets.
* TcpBase ensures the threads are cleaned up/joined when `cleanup()` is called
  * _Note:_ both threads will exit when `setExitCode(true)` is used
