#pragma once

#include <cstddef>
#include <iostream>
#include <cstdint>
#include <array>
#include <cstring>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "PacketsIO.h"
#include "ThreadPool.h"
#include "Utils.h"
#include "TickLoop.h"
#include "NetMacros.h"
#include "NetDataTypes.h"

namespace Networking
{
    class Client;
    class Server;

    using Port = unsigned short;

    template <typename TNetObject>
    class NetObjectInitializer
    {
        NetRegistry &registry;

    public:
        explicit NetObjectInitializer(NetRegistry &registry) : registry(registry) {}

        template <typename TInitial>
        TNetObject operator=(TInitial &&initial) const { return TNetObject{registry, std::forward<TInitial>(initial)}; }
        operator TNetObject() const { return TNetObject{registry}; }
    };

    class NetReceiverSenderBase
    {
        NetRegistry &registry;

    public:
        const bool isServer;

        NetReceiverSenderBase(const NetReceiverSenderBase &) = delete;
        virtual ~NetReceiverSenderBase() = default;

        explicit NetReceiverSenderBase(NetRegistry &registry, bool isServer) : registry(registry), isServer(isServer)
        {
        }
    };

    template <bool TIsServer>
    class NetReceiverSender : public NetReceiverSenderBase
    {
    protected:
        NET_TYPES(TIsServer);
        NET_REGISTRY();

    public:
        static constexpr bool IsServer = TIsServer;

        NetReceiverSender(const NetReceiverSender &) = delete;
        virtual ~NetReceiverSender() = default;

        explicit NetReceiverSender() : NetReceiverSenderBase(GET_NET_REGISTRY(), TIsServer)
        {
        }
    };
}
