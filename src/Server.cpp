#include "Server.h"

#include <cstring>
#include <string>

Server::Server() = default;

void Server::StartTickThread(const TickLoopDuration &tickStep, ThreadTask<int> &outTask)
{
    WRITE_LINE("Starting server tick thread...");

    outTask = StaticThreadPool().SubmitTask([tickStep, this]()
    {
        TickLoop tickLoop;
        TickLoopDuration deltaTime, duration;
        int result = 0;

        while (tickLoop.WaitForTick(deltaTime, duration, tickStep))
        {
            if ((result = OnTick()) != 0 || !IsRunning())
            tickLoop.Stop();
        }

        return result;
    });
}

int Server::OnTick()
{
    static const std::string response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 18\r\n"
        "Connection: close\r\n\r\n"
        "Hello from MUD!\n";

    clientSocket = accept(listenSocket, nullptr, nullptr);
    if (clientSocket == INVALID_SOCKET)
    {
        WRITE_ERROR("accept() failed: " << WSAGetLastError());
        return WSAGetLastError();
    }

    char buffer[1024]{};
    const int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived > 0)
        WRITE_LINE("Received request:\n"
                   << buffer);

    send(clientSocket, response.c_str(), static_cast<int>(response.size()), 0);
    closesocket(clientSocket);
    clientSocket = INVALID_SOCKET;

    return 0;
}

int Server::Run(Port port, TickLoopDuration tickStep, bool shouldWait)
{
    if (IsRunning())
        return -1;

    isRunningAtomic.store(true);
    Server::port = port;
    WSADATA wsaData{};

    const int startupResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (startupResult != 0)
    {
        WRITE_ERROR("WSAStartup failed: " << startupResult);
        return startupResult;
    }

    listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET)
    {
        WRITE_ERROR("socket() failed: " << WSAGetLastError());
        WSACleanup();
        return WSAGetLastError();
    }

    sockaddr_in service{};
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = INADDR_ANY;
    service.sin_port = htons(port);

    if (bind(listenSocket, reinterpret_cast<SOCKADDR *>(&service), sizeof(service)) == SOCKET_ERROR)
    {
        WRITE_ERROR("bind() failed: " << WSAGetLastError());
        Terminate();
        return WSAGetLastError();
    }

    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        WRITE_ERROR("listen() failed: " << WSAGetLastError());
        Terminate();
        return WSAGetLastError();
    }

    WRITE_LINE("Server listening on port " << port << "...");

    StartTickThread(tickStep, runningTask);

    return (shouldWait) ? Wait() : 0;
}

int Server::Terminate()
{
    if (!IsRunning())
        return -1;

    WRITE_LINE("Terminating server...");

    int result = 0;

    isRunningAtomic.store(false);
    result = runningTask.get();

    closesocket(listenSocket);
    WSACleanup();

    return result;
}