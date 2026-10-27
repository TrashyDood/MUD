#pragma once

#include <string>

#include <winsock2.h>
#include <ws2tcpip.h>

#include "ClientServerCommon.h"

namespace Networking
{
    using ClientId = unsigned short;

    class Client : public NetReceiverSender<false>
    {
        SOCKET socket_ = INVALID_SOCKET;

    public:
        static constexpr Port DefaultPort = Networking::DefaultPort;

        Client() = default;
        Client(const Client &) = delete;
        Client &operator=(const Client &) = delete;
        ~Client() override;

        bool IsConnected() const { return socket_ != INVALID_SOCKET; }
        SOCKET GetSocket() const { return socket_; }

        int Connect(const std::string &host = "127.0.0.1", Port port = DefaultPort);
        int Disconnect();
    };
}