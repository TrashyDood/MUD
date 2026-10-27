#include "Networking/NetData.h"

#include <cassert>
#include <limits>
#include <stdexcept>

using namespace Networking;

void Networking::NetRegistry::Register(NetObjBase &netObject)
{
    if (!vacantIds.empty())
    {
        const NetId netId = vacantIds.front();
        vacantIds.pop();
        netObjects[netId] = &netObject;
        netObject.netId = netId;
        ++objectCount;
        return;
    }

    if (netObjects.size() > static_cast<size_t>(std::numeric_limits<NetId>::max()))
        throw std::length_error("NetProp registry is full");

    const NetId netId = static_cast<NetId>(netObjects.size());
    netObjects.push_back(&netObject);
    netObject.netId = netId;
    ++objectCount;
}

void NetRegistry::Unregister(NetObjBase &netObject)
{
    if (netObject.netId < 0)
        return;

    const size_t index = static_cast<size_t>(netObject.netId);
    if (index >= netObjects.size() || netObjects[index] != &netObject)
        return;

    netObjects[index] = nullptr;
    vacantIds.push(netObject.netId);
    --objectCount;
}

Networking::NetRegistry::~NetRegistry()
{
    assert(objectCount == 0 && "NetRegistry destroyed with registered objects");
}

bool Networking::NetRegistry::TryFindById(NetId netId, NetObjBase *&outObject) const
{
    outObject = nullptr;
    if (netId < 0 || netId >= static_cast<NetId>(netObjects.size()))
        return false;

    const auto index = static_cast<size_t>(netId);

    outObject = netObjects[index];
    return outObject != nullptr;
}

bool Networking::NetRegistry::TryQueueSend(const NetId netId, const ReceiverId receiverId, const PendingSend &pendingSend)
{
    if (netId < 0 || netId >= static_cast<NetId>(netObjects.size()))
        return false;

    if (receiverId < 0)
    {
        for (auto &pendingSends : pendingSendsByReceiver)
            pendingSends.push(pendingSend);
        return true;
    }

    const size_t receiverIndex = static_cast<size_t>(receiverId);
    if (receiverIndex >= pendingSendsByReceiver.size())
        pendingSendsByReceiver.resize(receiverIndex + 1);

    pendingSendsByReceiver[receiverIndex].push(pendingSend);
    return true;
}

bool Networking::NetRegistry::TryPopPendingSend(ReceiverId receiverId, PendingSend &outSend)
{
    if (receiverId < 0)
        return false;

    const size_t receiverIndex = static_cast<size_t>(receiverId);
    if (receiverIndex >= pendingSendsByReceiver.size() || pendingSendsByReceiver[receiverIndex].empty())
        return false;

    outSend = std::move(pendingSendsByReceiver[receiverIndex].front());
    pendingSendsByReceiver[receiverIndex].pop();
    return true;
}

NetObjBase::NetObjBase(NetRegistry &registry, const NetPriority priority)
    : priority(priority), netRegistry(registry)
{
    registry.Register(*this);
}

NetObjBase::~NetObjBase()
{
    netRegistry.Unregister(*this);
}

bool Networking::NetObjBase::TrySend(ReceiverId receiverId, const PendingSend &pendingSend)
{
    return netRegistry.TryQueueSend(netId, receiverId, pendingSend);
}
