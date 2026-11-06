#include "Networking/NetData.h"

#include <limits>

using namespace Networking;

void NetRegistry::Register(NetObjBase &netObject)
{
    if (!vacantIds.empty())
    {
        const NetId netId = vacantIds.front();
        vacantIds.pop();
        registeredObjects[netId] = &netObject;
        netObject.netId = netId;
        ++objectCount;
        return;
    }

    if (registeredObjects.size() >= static_cast<size_t>(std::numeric_limits<NetId>::max()))
        throw std::length_error("NetProp registry is full");

    const NetId netId = static_cast<NetId>(registeredObjects.size());
    registeredObjects.push_back(&netObject);
    netObject.netId = netId;
    ++objectCount;
}

void NetRegistry::Unregister(NetObjBase &netObject)
{
    if (netObject.netId < 0)
        return;

    const size_t index = static_cast<size_t>(netObject.netId);
    if (index >= registeredObjects.size() || registeredObjects[index] != &netObject)
        return;

    registeredObjects[index] = nullptr;
    vacantIds.push(netObject.netId);
    --objectCount;
}

bool NetRegistry::TryFindById(const NetId netId, NetObjBase *&outObject) const
{
    outObject = nullptr;
    if (netId < 0 || netId >= static_cast<NetId>(registeredObjects.size()))
        return false;

    const auto index = static_cast<size_t>(netId);

    outObject = registeredObjects[index];
    return outObject != nullptr;
}