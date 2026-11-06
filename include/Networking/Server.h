#pragma once

#include <array>
#include <iostream>
#include <string>
#include <chrono>
#include <memory>

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
        SOCKET socket_ = INVALID_SOCKET;

    public:
        ConnectedClient(const ConnectedClient &) = delete;
        ConnectedClient &operator=(const ConnectedClient &) = delete;
        ~ConnectedClient();

        SOCKET GetSocket() const { return socket_; }

        explicit ConnectedClient(SOCKET socket) : socket_(socket) {}
    };

    class Server : public NetReceiverSender<true>
    {
        static constexpr ConnectionId MaxClientCount = 8;

        SOCKET listenSocket = INVALID_SOCKET;
        std::array<std::unique_ptr<ConnectedClient>, static_cast<std::size_t>(MaxClientCount)> connectedClients{};

        int AcceptIncomingClients();
        void ReceiveClientMessages(std::size_t clientIndex);
        void DisconnectClient(std::size_t clientIndex);
        SOCKET GetConnectionSocket(ConnectionId connectionId) const override
        {
            return connectionId < connectedClients.size() && connectedClients[connectionId]
                ? connectedClients[connectionId]->GetSocket()
                : INVALID_SOCKET;
        }
        int OnTick() override;

    public:
        int Listen(Port port = DefaultPort);
        int Terminate() override;
    };
}