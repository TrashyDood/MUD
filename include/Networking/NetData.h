#pragma once

#include <cstddef>
#include <queue>
#include <vector>

#include "NetMacros.h"

namespace Networking
{
    using NetId = int;
    using NetPriority = short;

    enum class NetAuthority
    {
        Server,
        Client,
        Shared,
        Local
    };

    class NetObjBase;

    class NetRegistry
    {
        friend class NetObjBase;

        std::vector<NetObjBase *> netObjects;
        std::queue<NetId> vacantIds;
        std::size_t objectCount = 0;
        std::queue<NetId> pendingSends, pendingReceives;

        void Register(NetObjBase &netObject);
        void Unregister(NetObjBase &netObject);
        bool MarkPendingSend(NetId id);

    public:
        size_t RegisteredObjectCount() const { return objectCount; }

        NetRegistry() = default;
        ~NetRegistry();
        
        bool TryFindById(NetId id, NetObjBase *&outObject) const;
    };

    class NetObjBase
    {
        friend class NetRegistry;

        NetId netId = INVALID_NET_ID;
        NetPriority priority = 0;
        bool isPending = false;

    protected:
        NET_REGISTRY_REF;

        bool MarkPending();

    public:
        NetObjBase(const NetObjBase &) = delete;
        NetObjBase &operator=(const NetObjBase &) = delete;
        NetObjBase(NetObjBase &&) = delete;
        NetObjBase &operator=(NetObjBase &&) = delete;

        NetId GetNetID() const { return netId; }
        NetPriority GetNetPriority() const { return priority; }
        bool IsRegistered() const { return netId >= 0; }
        NetRegistry &GetRegistry() const { return GET_NET_REGISTRY; }

        virtual bool ServerSide() const = 0;
        virtual bool CanSend() const = 0;
        virtual bool CanReceive() const = 0;

        NetObjBase(NetRegistry &registry, const NetPriority priority = 0);
        virtual ~NetObjBase();
    };

}