# pragma once

#include <iostream>
#include <string>
#include <chrono> 
#include <thread>

#include <winsock2.h>
#include <ws2tcpip.h>

#include "ClientServerCommon.h"

#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#endif

static constexpr Port DefaultPort = 8080;
using ServerTask = ThreadTask<int>;

class Server
{
    bool isRunning;
    Port port = DefaultPort;
    SOCKET listenSocket = INVALID_SOCKET;
    SOCKET clientSocket = INVALID_SOCKET;
    ServerTask runningTask;

    void StartThread(const Clock::duration& tickRate, ServerTask& outTask);

public:
    const ServerTask& RunningTask() const { return runningTask; }
    bool IsRunning() const { return isRunning; }
    Port CurrentPort() const { return port; }
    int Wait() { return runningTask.get(); }
    
    Server();

    int Run(Port port = DefaultPort, Clock::duration tickRate = 20ms, bool shouldWait = false);
    bool Terminate();
};