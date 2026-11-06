#include "Networking/Server.h"

#include <algorithm>
#include <array>

using namespace Networking;

ConnectedClient::~ConnectedClient()
{
    if (socket_ != INVALID_SOCKET)
        closesocket(socket_);
}

void Server::DisconnectClient(const std::size_t clientIndex)
{
    ClearSendByteQueue(static_cast<ConnectionId>(clientIndex));
    ClearReceiveByteQueue(static_cast<ConnectionId>(clientIndex));
    connectedClients[clientIndex].reset();
}

int Server::AcceptIncomingClients()
{
    while (true)
    {
        SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET)
        {
            const int error = WSAGetLastError();
            if (error == WSAEWOULDBLOCK)
                return 0;

            WRITE_ERROR("accept() failed: " << error);
            return error;
        }

        const auto freeClient = std::find(connectedClients.begin(), connectedClients.end(), nullptr);
        if (freeClient == connectedClients.end())
        {
            closesocket(clientSocket);
            continue;
        }

        const auto clientIndex = static_cast<std::size_t>(freeClient - connectedClients.begin());
        u_long nonBlocking = 1;
        if (ioctlsocket(clientSocket, FIONBIO, &nonBlocking) == SOCKET_ERROR)
        {
            WRITE_ERROR("ioctlsocket() failed for client: " << WSAGetLastError());
            closesocket(clientSocket);
            continue;
        }

        *freeClient = std::make_unique<ConnectedClient>(clientSocket);
        WRITE_LINE("Client connected in slot " << clientIndex);
    }

    return 0;
}

void Server::ReceiveClientMessages(const std::size_t clientIndex)
{
    bool peerClosed = false;
    const int receiveError = ReceiveBytes(static_cast<ConnectionId>(clientIndex), peerClosed);

    if (!peerClosed && receiveError == 0)
        return;

    DisconnectClient(clientIndex);
    if (peerClosed)
        WRITE_LINE("Client disconnected from slot " << clientIndex);
    else
        WRITE_ERROR("recv() failed for client in slot " << clientIndex << ": " << receiveError);
}

int Server::OnTick()
{
    const int acceptResult = AcceptIncomingClients();
    if (acceptResult != 0)
        return acceptResult;

    for (std::size_t clientIndex = 0; clientIndex < connectedClients.size(); ++clientIndex)
    {
        if (connectedClients[clientIndex])
        {
            ReceiveClientMessages(clientIndex);
            if (connectedClients[clientIndex])
            {
                const int sendError = FlushSendByteQueue(static_cast<ConnectionId>(clientIndex));
                if (sendError != 0)
                {
                    WRITE_ERROR("send() failed for client in slot " << clientIndex << ": " << sendError);
                    DisconnectClient(clientIndex);
                }
            }
        }
    }

    return 0;
}

int Server::Listen(const Port port)
{
    if (IsRunning())
        return -1;

    WSADATA wsaData{};

    const int startupResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (startupResult != 0)
    {
        WRITE_ERROR("WSAStartup failed: " << startupResult);
        return startupResult;
    }

    const auto fail = [this](const int error)
    {
        if (listenSocket != INVALID_SOCKET)
        {
            closesocket(listenSocket);
            listenSocket = INVALID_SOCKET;
        }
        WSACleanup();
        return error;
    };

    listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET)
    {
        const int error = WSAGetLastError();
        WRITE_ERROR("socket() failed: " << error);
        return fail(error);
    }

    sockaddr_in service{};
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = INADDR_ANY;
    service.sin_port = htons(port);

    if (bind(listenSocket, reinterpret_cast<SOCKADDR *>(&service), sizeof(service)) == SOCKET_ERROR)
    {
        const int error = WSAGetLastError();
        WRITE_ERROR("bind() failed: " << error);
        return fail(error);
    }

    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        const int error = WSAGetLastError();
        WRITE_ERROR("listen() failed: " << error);
        return fail(error);
    }

    u_long nonBlocking = 1;
    if (ioctlsocket(listenSocket, FIONBIO, &nonBlocking) == SOCKET_ERROR)
    {
        const int error = WSAGetLastError();
        WRITE_ERROR("ioctlsocket() failed for listener: " << error);
        return fail(error);
    }

    WRITE_LINE("Server listening on port " << port << "...");
    return 0;
}

int Server::Terminate()
{
    if (listenSocket == INVALID_SOCKET)
        return -1;

    WRITE_LINE("Terminating server...");

    const int result = NetReceiverSender<true>::Terminate();

    for (std::size_t i = 0; i < connectedClients.size(); ++i)
        DisconnectClient(i);

    closesocket(listenSocket);
    listenSocket = INVALID_SOCKET;
    WSACleanup();

    return result;
}