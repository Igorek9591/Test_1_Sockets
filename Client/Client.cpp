#include "Client.h"
#include <iostream>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

Client::Client(const char* ip, int port, int t) : timeout(t) {
    tcp = socket(AF_INET, SOCK_STREAM, 0);
    udp = socket(AF_INET, SOCK_DGRAM, 0);

    int flag = 1;
    setsockopt(tcp, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));

    setsockopt(tcp, SOL_SOCKET, SO_RCVTIMEO,
        (char*)&timeout, sizeof(timeout));

    sockaddr_in local{};
    local.sin_family = AF_INET;
    local.sin_port = htons(0);
    local.sin_addr.s_addr = INADDR_ANY;

    if (bind(udp, (sockaddr*)&local, sizeof(local)) < 0) {
        std::cout << "UDP bind failed\n";
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &serverAddr.sin_addr);

    connectWithRetry();
}

Client::Client(const char* ip, int port)
    : Client(ip, port, 1000) 
{
}

void Client::connectWithRetry() {
    while (connect(tcp, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cout << "Retrying to connect\n";
        Sleep(1000);
    }
}

uint32_t Client::getFileSize(std::ifstream& file) {
    file.seekg(0, std::ios::end);
    uint32_t size = file.tellg();
    file.seekg(0);
    return size;
}

std::vector<Packet> Client::splitFile(const char* fileName, uint32_t& fileSize) {
    std::ifstream file(fileName, std::ios::binary);
    fileSize = getFileSize(file);

    std::vector<Packet> packets;
    uint32_t id = 0;

    while (!file.eof()) {
        Packet pkt{};
        pkt.id = id++;
        file.read(pkt.data, MAX_DATA);
        pkt.size = file.gcount();
        pkt.islast = file.peek() == EOF;
        packets.push_back(pkt);
    }

    return packets;
}

void Client::sendMetadata(const char* fileName, uint32_t fileSize, int udpPort) {
    uint32_t nameLen = strlen(fileName);

    send(tcp, (char*)&nameLen, sizeof(nameLen), 0);
    send(tcp, fileName, nameLen, 0);
    send(tcp, (char*)&fileSize, sizeof(fileSize), 0);
    send(tcp, (char*)&udpPort, sizeof(udpPort), 0);
}

void Client::sendPackets(std::vector<Packet>& packets, int udpPort) {
    sockaddr_in udpAddr = serverAddr;
    udpAddr.sin_port = htons(udpPort);


    for (size_t i = 0; i < packets.size(); i++) {
        std::cout << "Sending packet " << packets[i].id << "\n";

        while (true) {
            std::cout << "Sending packet " << packets[i].id << "\n";

            sendto(udp, (char*)&packets[i], sizeof(Packet), 0,
                (sockaddr*)&udpAddr, sizeof(udpAddr));

            uint32_t ack;
            int r = recv(tcp, (char*)&ack, sizeof(ack), 0);

            if (r > 0 && ack == packets[i].id) {
                std::cout << "ACK received: " << ack << "\n";
                break;
            }

            if (r == SOCKET_ERROR && WSAGetLastError() == WSAETIMEDOUT) {
                std::cout << "Timeout, resend packet " << packets[i].id << "\n";
                continue;
            }

            std::cout << "recv error\n";
        }
    }
}

void Client::sendFile(const char* fileName, int udpPort) {
    uint32_t fileSize;
    auto packets = splitFile(fileName, fileSize);

    sendMetadata(fileName, fileSize, udpPort);
    sendPackets(packets, udpPort);
    send(tcp, "END", 3, 0);
    std::cout << "Transfer complete\n";
}

Client::~Client() {
    closesocket(tcp);
    closesocket(udp);
}