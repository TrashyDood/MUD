#include "Networking/NetObj.h"

NetObj::NetObj(NetRegistry &registry, const NetPriority priority)
    : priority(priority)
{
    SetRegistry(&registry);
}


void NetObj::SetRegistry(NetRegistry *newRegistry)
{
    if (registry == newRegistry)
        return;

    if (registry) registry->Unregister(*this);
    if (newRegistry) newRegistry->Register(*this);

    registry = newRegistry;
}

NetObj::~NetObj()
{
    SetRegistry(nullptr);
}