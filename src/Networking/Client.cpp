#include "Networking/Client.h"

#include <string>

using namespace Networking;

int Client::OnTick()
{
    if (!IsConnected())
        return 0;

    bool peerClosed = false;
    const int receiveError = ReceiveBytes(0, peerClosed);

    if (peerClosed || receiveError != 0)
    {
        const int disconnectError = Disconnect();
        if (disconnectError != 0)
            WRITE_ERROR("Disconnect cleanup failed: " << disconnectError);

        if (peerClosed)
            return WSAECONNRESET;

        WRITE_ERROR("recv() failed: " << receiveError);
        return receiveError;
    }

    const int sendError = FlushSendByteQueue(0);
    if (sendError != 0)
    {
        const int disconnectError = Disconnect();
        if (disconnectError != 0)
            WRITE_ERROR("Disconnect cleanup failed: " << disconnectError);
        WRITE_ERROR("send() failed: " << sendError);
        return sendError;
    }

    return 0;
}

int Client::Disconnect()
{
    ClearSendByteQueue(0);
    ClearReceiveByteQueue(0);
    if (!IsConnected())
        return 0;

    if (closesocket(socket_) == SOCKET_ERROR)
        return WSAGetLastError();

    socket_ = INVALID_SOCKET;

    if (WSACleanup() == SOCKET_ERROR)
        return WSAGetLastError();

    return 0;
}

int Client::Connect(const std::string &host, const Port port)
{
    if (IsConnected())
        return WSAEISCONN;

    const std::string service = std::to_string(port);
    WSADATA wsaData{};
    const int startupResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (startupResult != 0)
        return startupResult;

    addrinfo hints{};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    addrinfo *addresses = nullptr;
    const int lookupResult = getaddrinfo(host.c_str(), service.c_str(), &hints, &addresses);
    if (lookupResult != 0)
    {
        WSACleanup();
        return lookupResult;
    }

    int result = WSAECONNREFUSED;
    for (addrinfo *address = addresses; address != nullptr; address = address->ai_next)
    {
        SOCKET candidate = socket(address->ai_family, address->ai_socktype, address->ai_protocol);
        if (candidate == INVALID_SOCKET)
        {
            result = WSAGetLastError();
            continue;
        }

        if (connect(candidate, address->ai_addr, static_cast<int>(address->ai_addrlen)) == 0)
        {
            u_long nonBlocking = 1;
            if (ioctlsocket(candidate, FIONBIO, &nonBlocking) == 0)
            {
                socket_ = candidate;
                result = 0;
                break;
            }

            result = WSAGetLastError();
        }
        else
            result = WSAGetLastError();
        closesocket(candidate);
    }

    freeaddrinfo(addresses);
    if (!IsConnected())
    {
        WSACleanup();
        return result;
    }

    return 0;
}

int Client::Terminate()
{
    const int tickResult = NetReceiverSender<false>::Terminate();
    const int disconnectResult = Disconnect();
    return disconnectResult != 0 ? disconnectResult : tickResult;
}
