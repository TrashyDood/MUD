#include "../include/Server.h"

#include <cstring>
#include <string>

Server::Server() = default;

int Server::Run(Port port)
{
    if (isRunning) return 0;

    isRunning = true;
    Server::port = port;
    WSADATA wsaData{};

    const int startupResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (startupResult != 0)
    {
        std::cerr << "WSAStartup failed: " << startupResult << '\n';
        return startupResult;
    }

    listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET)
    {
        WRITE_ERROR("socket() failed: " << WSAGetLastError());
        //std::cerr << "socket() failed: " << WSAGetLastError() << '\n';

        WSACleanup();
        return WSAGetLastError();
    }

    sockaddr_in service{};
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = INADDR_ANY;
    service.sin_port = htons(port);

    if (bind(listenSocket, reinterpret_cast<SOCKADDR*>(&service), sizeof(service)) == SOCKET_ERROR)
    {
        WRITE_ERROR("bind() failed: " << WSAGetLastError());
        //std::cerr << "bind() failed: " << WSAGetLastError() << '\n';

        Terminate();
        return WSAGetLastError();
    }

    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        WRITE_ERROR("listen() failed: " << WSAGetLastError());
        //std::cerr << "listen() failed: " << WSAGetLastError() << '\n';

        Terminate();
        return WSAGetLastError();
    }

    OnRun();
    return 0;
}

bool Server::Terminate()
{
    if (!isRunning) return false;

    closesocket(listenSocket);
    WSACleanup();
    isRunning = false;

    return true;
}

void Server::OnRun()
{
    WRITELINE("Server listening on port " << port << "...");
    //std::cout << "Server listening on port " << DefaultPort << "...\n";

    while (isRunning)
    {
        clientSocket = accept(listenSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET)
        {
            WRITE_ERROR("accept() failed: " << WSAGetLastError());
            //std::cerr << "accept() failed: " << WSAGetLastError() << '\n';
            break;
        }

        char buffer[1024]{};
        const int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived > 0)
        {
            WRITELINE("Received request:\n" << buffer);
            //std::cout << "Received request:\n" << buffer << '\n';
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
}