#pragma once

#include <cstddef>
#include <iostream>
#include <cstdint>
#include <array>
#include <algorithm>
#include <cstring>
#include <deque>
#include <limits>
#include <mutex>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <atomic>
#include <vector>
#include <winsock2.h>

#include "ThreadPool.h"
#include "NetUtils.h"
#include "TickLoop.h"
#include "NetMacros.h"
#include "NetDataTypes.h"

namespace Networking
{
    class Client;
    class Server;

    using Port = unsigned short;
    using ConnectionId = ReceiverId;

    inline constexpr Port DefaultPort = 8080;
    inline constexpr TickLoop::Duration DefaultTickStep = 33ms;
    inline constexpr size_t MessageLength = 4096;

    using ByteBuffer = std::array<Byte, MessageLength>;
    using MessageParser = std::function<void(const ByteBuffer &, std::size_t)>;

    inline std::array<MessageParser, 256> MessageParsers{};

    class NetReceiverSenderBase : public NetRegistry
    {
        std::atomic<bool> isRunning_Atomic = false;
        ThreadTask<int> runningTask;
        std::mutex sendByteQueuesMutex;
        std::vector<std::deque<Byte>> sendByteQueues;
        std::vector<std::deque<Byte>> receiveByteQueues;

        int StopTickThread();
        virtual SOCKET GetConnectionSocket(ConnectionId connectionId) const = 0;

    public:
        const bool isServer;

        ThreadTask<int> &RunningTask() { return runningTask; }
        bool IsRunning() const { return isRunning_Atomic.load(); }
        int Wait() { return runningTask.get(); }

        NetReceiverSenderBase(const NetReceiverSenderBase &) = delete;
        NetReceiverSenderBase &operator=(const NetReceiverSenderBase &) = delete;
        virtual ~NetReceiverSenderBase() = default;

        explicit NetReceiverSenderBase(bool isServer) : isServer(isServer) {}

        int Run(TickLoop::Duration tickStep = DefaultTickStep);
        bool SendBytes(ConnectionId connectionId, const void *data, std::size_t size);

        virtual int Terminate() { return StopTickThread(); }

    protected:
        void StartTickThread(const TickLoop::Duration &tickStep);
        int ReceiveBytes(ConnectionId connectionId, bool &outPeerClosed);
        int FlushSendByteQueue(ConnectionId connectionId);
        void ClearSendByteQueue(ConnectionId connectionId);
        void ClearReceiveByteQueue(ConnectionId connectionId);

        virtual void PreTick() {}
        virtual int OnTick() = 0;
        virtual void PostTick() {}

        int OnBytesReceived(ConnectionId connectionId, const ByteBuffer &messageBuffer, std::size_t size);

        void OnNewNetCall(NetCall &) override {}
    };

    NET_ROLE_ARGS
    class NetReceiverSender : public NetReceiverSenderBase
    {
        NET_TYPES;

    public:
        static constexpr bool IsServer = TIsServer;

        NetReceiverSender(const NetReceiverSender &) = delete;
        NetReceiverSender &operator=(const NetReceiverSender &) = delete;
        virtual ~NetReceiverSender() = default;

        explicit NetReceiverSender() : NetReceiverSenderBase(TIsServer) {}
    };
}
