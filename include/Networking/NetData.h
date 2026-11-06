#pragma once

#include "NetUtils.h"

#include <cstddef>
#include <queue>
#include <tuple>
#include <utility>
#include <vector>
#include <stdexcept>
#include <cassert>

namespace Networking
{
    class NetObjBase;
    class NetRegistry;

    using NetId = int;
    using ReceiverId = short;
    using NetPriority = short;
    using NetRegistryId = unsigned short;
    using MessageType = unsigned char;

    inline constexpr ReceiverId DefaultReceiverId = -1;
    inline constexpr NetId InvalidNetId = -1;

    template <bool TIsServer>
    inline NetRegistry &StaticNetRegistry()
    {
        static NetRegistry instance{};
        return instance;
    }

    enum class NetAuthority
    {
        Server,
        Client,
        Shared,
        Local
    };

    struct NetCall
    {
        const NetObjBase *netObject;
        const ReceiverId receiverId;
        const MessageType messageType;
        std::vector<Byte> byteData{};

        bool IsValid() const { return netObject; }
    };

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

    class NetRegistry
    {
        friend class NetObjBase;

        std::size_t objectCount = 0;
        std::vector<NetObjBase *> registeredObjects{};
        std::queue<NetId> vacantIds{};

        void Register(NetObjBase &netObject);
        void Unregister(NetObjBase &netObject);

    protected:
        virtual void OnNewNetCall(NetCall &netcall) = 0;

    public:
        size_t RegisteredObjectCount() const { return objectCount; }

        ~NetRegistry() { assert(objectCount == 0 && "NetRegistry destroyed with registered objects"); }
        NetRegistry() = default;

        bool TryFindById(const NetId netId, NetObjBase *&outObject) const;
    };

    class NetObjBase
    {
        friend class NetRegistry;

        NetId netId = InvalidNetId;
        NetPriority netPriority = 0;

    protected:
        NetRegistry &netRegistry;

        template <typename... TToSend>
        bool TrySendData(const ReceiverId receiverId, const MessageType messageType, const TToSend &...toSend)
        {
            if (!GetCanSend()) return false;

            NetCall netCall{this, receiverId, messageType, std::vector<Byte>((sizeof(TToSend) + ...))};

            if (!PacketsIO::TryWriteBytesPackage(netCall.byteData, 0, toSend...))
                return false;

            netRegistry.OnNewNetCall(netCall);
            return true;
        }

    public:
        NetObjBase(const NetObjBase &) = delete;
        NetObjBase &operator=(const NetObjBase &) = delete;
        NetObjBase(NetObjBase &&) = delete;
        NetObjBase &operator=(NetObjBase &&) = delete;

        NetId GetNetID() const { return netId; }
        NetPriority GetNetPriority() const { return netPriority; }
        bool IsRegistered() const { return netId >= 0; }
        NetRegistry &GetRegistry() const { return netRegistry; }

        virtual bool GetIsServerSide() const = 0;
        virtual bool GetCanSend() const = 0;
        virtual bool GetCanReceive() const = 0;
        virtual bool GetToSendByteSize() const = 0;

        NetObjBase(NetRegistry &netRegistry, const NetPriority netPriority = 0)
            : netPriority(netPriority), netRegistry(netRegistry) { netRegistry.Register(*this); }

        virtual ~NetObjBase() { netRegistry.Unregister(*this); }
    };
}