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
        NET_REGISTRY_REF;
        const bool isServer;

    public:
        NetReceiverSenderBase(const NetReceiverSenderBase &) = delete;
        virtual ~NetReceiverSenderBase() = default;

        explicit NetReceiverSenderBase(NetRegistry &registry, bool isServer)
            : GET_NET_REGISTRY()(registry), isServer(isServer) {}
    };

    template <bool TIsServer>
    class NetReceiverSender : public NetReceiverSenderBase
    {
    protected:
        NET_TYPES(TIsServer);
        NET_REGISTRY();

        NET_PROP(test, int, NetAuthority::Shared);
        NET_FUNC(testFunc, void(NetId), NetAuthority::Shared) = [this](NetId callerId)
        {
            std::cout << "testFunc called! " << callerId << std::endl;
        };

    public:
        static constexpr bool IsServer = TIsServer;

        NetReceiverSender(const NetReceiverSender &) = delete;
        virtual ~NetReceiverSender() = default;

        explicit NetReceiverSender()
            : NetReceiverSenderBase(GET_NET_REGISTRY(), TIsServer) {}
    };
}
