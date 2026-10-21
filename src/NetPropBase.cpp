#include "NetPropBase.h"

void NetObj::SetRegistry(NetRegistry *registry)
{
    if (NetObj::registry)
        NetObj::registry->Unregister(*this);

    NetObj::registry = registry;

    if (NetObj::registry)
        NetObj::registry->Register(*this);
}

NetObj::NetObj(NetRegistry &registry)
{
    SetRegistry(&registry);
}

NetObj::~NetObj()
{
    SetRegistry(nullptr);
}