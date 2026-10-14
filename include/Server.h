# pragma once

#include <iostream>
#include <string>
#include <chrono> 
#include <thread>

#include <winsock2.h>
#include <ws2tcpip.h>

#include "../include/ClientServerCommon.h"

#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#endif

static constexpr Port DefaultPort = 8080;

class Server
{
    bool isRunning;
    Port port = DefaultPort;
    SOCKET listenSocket = INVALID_SOCKET;
    SOCKET clientSocket = INVALID_SOCKET;

public:
    bool IsRunning() const { return isRunning; }
    Port CurrentPort() const { return port; }

    Server();

    int Run(Port port = DefaultPort);
    bool Terminate();

private:
    void OnRun();
};