#include "NetRegistry.h"
#include "NetPropBase.h"

#include <limits>
#include <stdexcept>

void NetRegistry::Register(NetObj &netObj)
{
    if (!vacantIds.empty())
    {
        netObj.id = vacantIds.front();
        vacantIds.pop();
        netObjects[netObj.id] = &netObj;
        return;
    }

    if (netObjects.size() > static_cast<size_t>(std::numeric_limits<NetId>::max()))
        throw std::length_error("NetProp registry is full");

    netObj.id = static_cast<NetId>(netObjects.size());
    netObjects.push_back(&netObj);
}

void NetRegistry::Unregister(NetObj &netObj)
{
    if (!netObj.IsRegistered())
        return;

    const size_t index = static_cast<size_t>(netObj.id);
    if (index >= netObjects.size() || netObjects[index] != &netObj)
        return;

    netObjects[index] = nullptr;
    vacantIds.push(netObj.id);
    netObj.id = -1;
}

NetObj *NetRegistry::Find(NetId id) const
{
    if (id < 0)
        return nullptr;

    const auto index = (size_t)id;
    return index < netObjects.size() ? netObjects[index] : nullptr;
}