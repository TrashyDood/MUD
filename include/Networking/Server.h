#pragma once

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

namespace Networking
{
    class ConnectedClient
    {
        SOCKET socket = INVALID_SOCKET;

    public:
        ConnectedClient(const ConnectedClient &) = delete;
        ConnectedClient &operator=(const ConnectedClient &) = delete;
        virtual ~ConnectedClient() = default;

        SOCKET GetSocket() const { return socket; }

        ConnectedClient(SOCKET socket)
            : socket(socket) {}
    };

    class Server : public NetReceiverSender<true>
    {
        static constexpr uint16_t MaxClientCount = 8;
        static constexpr TickLoop::Duration ServerDefaultTickStep = 33ms;

        std::atomic<bool> isRunning_Atomic = false;
        Port port = DefaultPort;
        SOCKET listenSocket = INVALID_SOCKET;
        SOCKET clientSocket = INVALID_SOCKET;
        ThreadTask<int> runningTask;

        void StartTickThread(const TickLoop::Duration &tickStep, ThreadTask<int> &outTask);
        int OnTick();

    public:
        constexpr ThreadTask<int> &RunningTask() { return runningTask; }
        bool IsRunning() const { return isRunning_Atomic.load(); }

        int Wait() { return runningTask.get(); }
        int Run(Port port = DefaultPort, TickLoop::Duration tickStep = ServerDefaultTickStep, bool shouldWait = false);
        int Terminate();
    };
}