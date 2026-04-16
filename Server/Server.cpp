#include "Server.h"
#include <iostream>
#include <thread>
#include <fstream>

#pragma comment(lib, "ws2_32.lib")

Server::Server(const char* ip, int port, const std::string& d)
    : dir(d)
{
    listenSock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &addr.sin_addr);

    bind(listenSock, (sockaddr*)&addr, sizeof(addr));
    listen(listenSock, SOMAXCONN);
}

void Server::start() {
    while (true) {
        std::cout << "Waiting for client\n";

        SOCKET client = accept(listenSock, nullptr, nullptr);

        if (client == INVALID_SOCKET)
            continue;
    
        std::cout << "Client connected\n";
        handleClient(client);
    }
}
void Server::handleClient(SOCKET clientSock) {
    char fileName[256];
    uint32_t fileSize;
    uint16_t udpPort;

    std::cout << "Handling client\n";

    uint32_t nameLen;
    recv(clientSock, (char*)&nameLen, sizeof(nameLen), 0);

    recv(clientSock, fileName, nameLen, 0);
    fileName[nameLen] = '\0';

    recv(clientSock, (char*)&fileSize, sizeof(fileSize), 0);
    recv(clientSock, (char*)&udpPort, sizeof(udpPort), 0);

    SOCKET udp = socket(AF_INET, SOCK_DGRAM, 0);

    std::cout << "File: " << fileName
        << " Size: " << fileSize
        << " UDP port: " << udpPort << "\n";


    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(udpPort);
    addr.sin_addr.s_addr = INADDR_ANY;
    bind(udp, (sockaddr*)&addr, sizeof(addr));

    std::map<uint32_t, std::vector<char>> buffer;
    uint32_t receivedBytes = 0;

    while (true) {
        std::cout << "Waiting UDP...\n";
        Packet pkt{};
        sockaddr_in from;
        int fromLen = sizeof(from);

        int bytes = recvfrom(udp, (char*)&pkt, sizeof(pkt), 0,
            (sockaddr*)&from, &fromLen);

        if (bytes <= 0) continue;

        std::cout << "Received packet " << pkt.id
            << " size=" << pkt.size << "\n";


        if (!buffer.count(pkt.id)) {
            buffer[pkt.id] = std::vector<char>(pkt.data, pkt.data + pkt.size);
            receivedBytes += pkt.size;
        }

        send(clientSock, (char*)&pkt.id, sizeof(pkt.id), 0);

        if (receivedBytes >= fileSize || pkt.islast)
            break;
    }

    char endMsg[4] = {};
    recv(clientSock, endMsg, 3, 0);

    std::ofstream out(dir + "/" + fileName, std::ios::binary);

    for (auto& pair : buffer) {
        auto& chunk = pair.second;
        out.write(chunk.data(), chunk.size());
    }

    std::cout << "Saved: " << fileName << std::endl;
    std::cout << "Saving to: " << dir + "/" + fileName << "\n";

    closesocket(clientSock);
    closesocket(udp);
}

Server::~Server() {
    closesocket(listenSock);
}