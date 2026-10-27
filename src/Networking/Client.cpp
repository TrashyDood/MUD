#include "Networking/Client.h"

#include <string>

using namespace Networking;

Client::~Client()
{
    Disconnect();
}

int Client::Connect(const std::string &host, const Port port)
{
    if (IsConnected())
        return WSAEISCONN;

    WSADATA wsaData{};
    const int startupResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (startupResult != 0)
        return startupResult;

    addrinfo hints{};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    const std::string service = std::to_string(port);
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
            socket_ = candidate;
            result = 0;
            break;
        }

        result = WSAGetLastError();
        closesocket(candidate);
    }

    freeaddrinfo(addresses);
    if (!IsConnected())
        WSACleanup();

    return result;
}

int Client::Disconnect()
{
    if (!IsConnected())
        return 0;

    const int closeResult = closesocket(socket_);
    if (closeResult == SOCKET_ERROR)
        return WSAGetLastError();

    socket_ = INVALID_SOCKET;

    const int cleanupResult = WSACleanup();
    if (cleanupResult == SOCKET_ERROR)
        return WSAGetLastError();

    return 0;
}
