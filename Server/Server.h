#ifndef SERVER_H
#define SERVER_H

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#include <map>
#include <vector>

#pragma pack(push,1)
const int MAX_DATA = 1000;

struct Packet {
    uint32_t id;
    uint32_t size;
    uint8_t islast;
    char data[MAX_DATA];
};
#pragma pack(pop)

class Server {
private:
    SOCKET listenSock;
    std::string dir;

    void handleClient(SOCKET clientSock);

public:
    Server(const char* ip, int port, const std::string& d);
    Server(const char* ip, int port);

    void start();
    ~Server();
};

#endif