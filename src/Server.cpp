#include "../include/Server.h"

#include <cstring>
#include <iostream>
#include <string>

Server::Server() = default;

void Server::Run()
{
    WSADATA wsaData{};
    SOCKET listenSocket = INVALID_SOCKET;
    SOCKET clientSocket = INVALID_SOCKET;

    const int startupResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (startupResult != 0)
    {
        std::cerr << "WSAStartup failed: " << startupResult << '\n';
        return;
    }

    listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET)
    {
        std::cerr << "socket() failed: " << WSAGetLastError() << '\n';
        WSACleanup();
        return;
    }

    sockaddr_in service{};
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = INADDR_ANY;
    service.sin_port = htons(DefaultPort);

    if (bind(listenSocket, reinterpret_cast<SOCKADDR*>(&service), sizeof(service)) == SOCKET_ERROR)
    {
        std::cerr << "bind() failed: " << WSAGetLastError() << '\n';
        closesocket(listenSocket);
        WSACleanup();
        return;
    }

    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        std::cerr << "listen() failed: " << WSAGetLastError() << '\n';
        closesocket(listenSocket);
        WSACleanup();
        return;
    }

    std::cout << "MUD server listening on port " << DefaultPort << "...\n";

    while (true)
    {
        clientSocket = accept(listenSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET)
        {
            std::cerr << "accept() failed: " << WSAGetLastError() << '\n';
            break;
        }

        char buffer[1024]{};
        const int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived > 0)
        {
            std::cout << "Received request:\n" << buffer << '\n';
        }

        const std::string response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 18\r\n"
            "Connection: close\r\n\r\n"
            "Hello from MUD!\n";

        send(clientSocket, response.c_str(), static_cast<int>(response.size()), 0);
        closesocket(clientSocket);
        clientSocket = INVALID_SOCKET;
    }

    closesocket(listenSocket);
    WSACleanup();
}