#include "server.h"
#include <WinSock2.h>
#include <iostream>
#include <cstdlib>

int main(int argc, char* argv[]) {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    Server server(argv[1], atoi(argv[2]), argv[3]);
    server.start();

    WSACleanup();
}