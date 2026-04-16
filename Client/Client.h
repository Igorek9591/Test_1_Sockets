#ifndef CLIENT_H
#define CLIENT_H

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <vector>
#include <map>
#include <fstream>

#pragma pack(push,1)
const int MAX_DATA = 1000;

struct Packet {
    uint32_t id;
    uint32_t size;
    uint8_t islast;
    char data[MAX_DATA];
};
#pragma pack(pop)

class Client {
private:
    SOCKET tcp;
    SOCKET udp;
    sockaddr_in serverAddr{};
    int timeout;

    void connectWithRetry();
    uint32_t getFileSize(std::ifstream& file);

public:
    Client(const char* ip, int port, int t);

    Client(const char* ip, int port);

    std::vector<Packet> splitFile(const char* fileName, uint32_t& fileSize);
    void sendMetadata(const char* fileName, uint32_t fileSize, int udpPort);
    void sendPackets(std::vector<Packet>& packets, int udpPort);
    void sendFile(const char* fileName, int udpPort);

    ~Client();
};

#endif