# udp_socket
Lab using udp socket client/server implementation

## Requirements
```sh
sudo apt-get update && sudo apt install -y make gcc libssl-dev

```
## Client
Start client that will send messages to server `127.0.0.1` and port `1234`
```sh
cd client/
make
./client 127.0.0.1 1234
```

## Server
Build and start server on local host on port 1234
```sh
cd server/
make
./Start_server.sh
```
