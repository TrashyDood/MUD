#include "Networking/NetData.h"

#include <cassert>
#include <limits>
#include <stdexcept>

using namespace Networking;

void Networking::NetRegistry::Register(NetObjBase &netObject)
{
    if (!vacantIds.empty())
    {
        const NetId id = vacantIds.front();
        vacantIds.pop();
        netObjects[id] = &netObject;
        netObject.netId = id;
        ++objectCount;
        return;
    }

    if (netObjects.size() > static_cast<size_t>(std::numeric_limits<NetId>::max()))
        throw std::length_error("NetProp registry is full");

    const NetId id = static_cast<NetId>(netObjects.size());
    netObjects.push_back(&netObject);
    netObject.netId = id;
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

bool Networking::NetRegistry::MarkPendingSend(NetId id)
{
    if (id < 0 || id >= static_cast<NetId>(netObjects.size()))
        return false;

    pendingSends.push(id);
    return true;
}

Networking::NetRegistry::~NetRegistry()
{
    assert(objectCount == 0 && "NetRegistry destroyed with registered objects");
}

bool Networking::NetRegistry::TryFindById(NetId id, NetObjBase *&outObject) const
{
    if (id < 0)
    {
        outObject = nullptr;
        return false;
    }

    const auto index = static_cast<size_t>(id);
    if (index >= netObjects.size())
    {
        outObject = nullptr;
        return false;
    }

    outObject = netObjects[index];
    return outObject != nullptr;
}

NetObjBase::NetObjBase(NetRegistry &registry, const NetPriority priority)
    : priority(priority), GET_NET_REGISTRY(registry)
{
    registry.Register(*this);
}

NetObjBase::~NetObjBase()
{
    GET_NET_REGISTRY.Unregister(*this);
}

bool Networking::NetObjBase::MarkPending()
{
    if (isPending || !GET_NET_REGISTRY.MarkPendingSend(netId))
        return false;

    isPending = true;

    return true;
}
