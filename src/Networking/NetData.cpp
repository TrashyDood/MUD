#include "Networking/NetData.h"

#include <limits>
#include <stdexcept>

using namespace Networking;

void Networking::NetRegistry::Register(NetObj &netObject)
{
    if (!vacantIds.empty())
    {
        const NetId id = vacantIds.front();
        vacantIds.pop();
        netObjects[id] = &netObject;
        netObject.id = id;
    }

    if (netObjects.size() > static_cast<size_t>(std::numeric_limits<NetId>::max()))
        throw std::length_error("NetProp registry is full");

    const NetId id = static_cast<NetId>(netObjects.size());
    netObjects.push_back(&netObject);
    netObject.id = id;
}

void NetRegistry::Unregister(NetObj &netObject)
{
    if (netObject.id < 0)
        return;

    const size_t index = static_cast<size_t>(netObject.id);
    if (index >= netObjects.size() || netObjects[index] != &netObject)
        return;

    netObjects[index] = nullptr;
    vacantIds.push(netObject.id);
    netObject.id = -1;
}

NetObj *NetRegistry::Find(NetId id) const
{
    if (id < 0)
        return nullptr;

    const auto index = static_cast<size_t>(id);
    return index < netObjects.size() ? netObjects[index] : nullptr;
}

NetObj::NetObj(NetRegistry &registry, const NetPriority priority)
    : registry(registry), priority(priority)
{
    registry.Register(*this);
}

NetObj::~NetObj()
{
    registry.Unregister(*this);
}