# pragma once

#include <iostream>
#include <string>
#include <chrono> 
#include <thread>
#include <atomic>
#include <vector>

#include <winsock2.h>
#include <ws2tcpip.h>

#include "ClientServerCommon.h"

#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#endif

static constexpr Port DefaultPort = 8080;
static constexpr unsigned char MaxClientCount = 8;
static constexpr TickLoopDuration ServerDefaultTickStep = 50ms;

class Server
{
    std::atomic<bool> isRunningAtomic;
    Port port = DefaultPort;
    SOCKET listenSocket = INVALID_SOCKET;
    SOCKET clientSocket = INVALID_SOCKET;
    ThreadTask<int> runningTask;
    std::vector<SOCKET> clientSockets;

    void StartTickThread(const TickLoopDuration& tickStep, ThreadTask<int>& outTask);
    int OnTick();

public:
    const ThreadTask<int>& RunningTask() const { return runningTask; }
    bool IsRunning() const { return isRunningAtomic.load(); }
    int Wait() { return runningTask.get(); }
    
    Server();

    int Run(Port port = DefaultPort, TickLoopDuration tickStep = ServerDefaultTickStep, bool shouldWait = false);
    int Terminate();
};