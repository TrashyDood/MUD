#pragma once

#include <any>
#include <cstddef>
#include <queue>
#include <utility>
#include <vector>

namespace Networking
{
    class NetObjBase;
    class NetRegistry;

    using NetId = int;
    using NetPriority = short;
    using ReceiverId = short;

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

    struct PendingSend
    {
        NetId netId;
        std::any payload;

        template <typename TPayload>
        static PendingSend Create(const NetId netId, const TPayload &payload) { return PendingSend{netId, payload}; }
    };

    class NetRegistry
    {
        friend class NetObjBase;

        std::vector<NetObjBase *> netObjects{};
        std::queue<NetId> vacantIds{};
        std::size_t objectCount = 0;
        std::vector<std::queue<PendingSend>> pendingSendsByReceiver{1};

        void Register(NetObjBase &netObject);
        void Unregister(NetObjBase &netObject);

    public:
        size_t RegisteredObjectCount() const { return objectCount; }

        ~NetRegistry();
        NetRegistry() = default;

        bool TryQueueSend(const NetId netId, const ReceiverId receiverId, const PendingSend &pendingSend);

        bool TryPopPendingSend(ReceiverId receiverId, PendingSend &outSend);
        bool TryFindById(NetId netId, NetObjBase *&outObject) const;
    };

    class NetObjBase
    {
        friend class NetRegistry;

        NetId netId = InvalidNetId;
        NetPriority priority = 0;

    protected:
        NetRegistry &netRegistry;

        bool TrySend(ReceiverId receiverId, const PendingSend &pendingSend);

    public:
        NetObjBase(const NetObjBase &) = delete;
        NetObjBase &operator=(const NetObjBase &) = delete;
        NetObjBase(NetObjBase &&) = delete;
        NetObjBase &operator=(NetObjBase &&) = delete;

        NetId GetNetID() const { return netId; }
        NetPriority GetNetPriority() const { return priority; }
        bool IsRegistered() const { return netId >= 0; }
        NetRegistry &GetRegistry() const { return netRegistry; }

        virtual bool ServerSide() const = 0;
        virtual bool CanSend() const = 0;
        virtual bool CanReceive() const = 0;

        NetObjBase(NetRegistry &registry, const NetPriority priority = 0);
        virtual ~NetObjBase();
    };
}