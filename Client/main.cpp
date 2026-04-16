#include "Client.h"
#include <WinSock2.h>
#include <iostream>


int main(int argc, char* argv[]) {
    if (argc != 6) return 1;

    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    Client client(argv[1], atoi(argv[2]), atoi(argv[5]));
    client.sendFile(argv[4], atoi(argv[3]));

    WSACleanup();
    return 0;
}