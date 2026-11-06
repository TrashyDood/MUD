#include "Networking/ClientServerCommon.h"

#include <stdexcept>

using namespace Networking;

bool NetReceiverSenderBase::SendBytes(const ConnectionId connectionId, const void *data, const std::size_t size)
{
    if (connectionId < 0 || data == nullptr || GetConnectionSocket(connectionId) == INVALID_SOCKET)
        return false;

    if (size == 0)
        return true;

    const auto *bytes = static_cast<const Byte *>(data);
    std::lock_guard lock(sendByteQueuesMutex);
    const auto index = static_cast<std::size_t>(connectionId);
    if (index >= sendByteQueues.size())
        sendByteQueues.resize(index + 1);
    auto &sendQueue = sendByteQueues[index];
    sendQueue.insert(sendQueue.end(), bytes, bytes + size);
    return true;
}

int NetReceiverSenderBase::FlushSendByteQueue(const ConnectionId connectionId)
{
    if (connectionId < 0)
        return WSAEINVAL;

    const SOCKET socket = GetConnectionSocket(connectionId);
    if (socket == INVALID_SOCKET)
        return WSAENOTSOCK;

    std::lock_guard lock(sendByteQueuesMutex);
    const auto index = static_cast<std::size_t>(connectionId);
    if (index >= sendByteQueues.size())
        return 0;

    auto &sendQueue = sendByteQueues[index];
    ByteBuffer sendBuffer{};
    while (!sendQueue.empty())
    {
        const auto toSend = std::min(sendQueue.size(), sendBuffer.size());
        std::copy_n(sendQueue.begin(), toSend, sendBuffer.begin());
        const int sent = send(socket, sendBuffer.data(), static_cast<int>(toSend), 0);

        if (sent > 0)
        {
            for (int i = 0; i < sent; ++i)
                sendQueue.pop_front();
            continue;
        }

        if (sent == 0)
            return WSAECONNRESET;

        const int error = WSAGetLastError();
        if (error == WSAEWOULDBLOCK)
            return 0;
        return error;
    }

    return 0;
}

void NetReceiverSenderBase::ClearSendByteQueue(const ConnectionId connectionId)
{
    if (connectionId < 0)
        return;

    std::lock_guard lock(sendByteQueuesMutex);
    const auto index = static_cast<std::size_t>(connectionId);
    if (index < sendByteQueues.size())
        sendByteQueues[index].clear();
}

void NetReceiverSenderBase::ClearReceiveByteQueue(const ConnectionId connectionId)
{
    if (connectionId < 0)
        return;

    const auto index = static_cast<std::size_t>(connectionId);
    if (index < receiveByteQueues.size())
        receiveByteQueues[index].clear();
}

int NetReceiverSenderBase::OnBytesReceived(const ConnectionId connectionId, const ByteBuffer &receivedChunk, const std::size_t size)
{
    if (connectionId < 0 || size > receivedChunk.size())
        return WSAEINVAL;

    const auto index = static_cast<std::size_t>(connectionId);
    if (index >= receiveByteQueues.size())
        receiveByteQueues.resize(index + 1);

    auto &receiveQueue = receiveByteQueues[index];
    receiveQueue.insert(
        receiveQueue.end(),
        receivedChunk.begin(),
        receivedChunk.begin() + size);
    return 0;
}

int NetReceiverSenderBase::ReceiveBytes(const ConnectionId connectionId, bool &outPeerClosed)
{
    outPeerClosed = false;
    const SOCKET socket = GetConnectionSocket(connectionId);
    if (socket == INVALID_SOCKET)
        return WSAENOTSOCK;

    ByteBuffer message{};

    while (true)
    {
        const int result = recv(socket, message.data(), static_cast<int>(message.size()), 0);
        if (result > 0)
        {
            const int receiveResult = OnBytesReceived(connectionId, message, static_cast<std::size_t>(result));
            if (receiveResult != 0)
                return receiveResult;
            continue;
        }

        if (result == 0)
        {
            outPeerClosed = true;
            return 0;
        }

        const int error = WSAGetLastError();
        return error == WSAEWOULDBLOCK ? 0 : error;
    }
}

void NetReceiverSenderBase::StartTickThread(const TickLoop::Duration &tickStep)
{
    WRITE_LINE("Starting " << (isServer ? "server" : "client") << " tick thread...");
    isRunning_Atomic.store(true);
    runningTask = StaticThreadPool().SubmitTask([tickStep, this]()
    {
        TickLoop tickLoop;
        int result = 0;

        while (tickLoop.WaitForTick(tickStep))
        {
            PreTick();
            result = OnTick();
            PostTick();

            if (result != 0 || !IsRunning())
                break;
        }

        tickLoop.Stop();
        isRunning_Atomic.store(false);
        return result;
    });
}

int NetReceiverSenderBase::Run(const TickLoop::Duration tickStep)
{
    if (IsRunning())
        return -1;

    StartTickThread(tickStep);
    return 0;
}

int NetReceiverSenderBase::StopTickThread()
{
    isRunning_Atomic.store(false);
    return runningTask.valid() ? runningTask.get() : 0;
}