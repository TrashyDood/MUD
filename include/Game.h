#pragma once

#include "Networking/ClientServerCommon.h"
#include "Networking/Server.h"
#include "Networking/Client.h"

template <bool TIsServer>
class Game_Shared : public NET_ROLE_SHARED(TIsServer)
{
    NET_ROLE_TYPES(NET_ROLE_SHARED(TIsServer));

protected:
    NET_PROP(test, int, NetAuthority::Shared);
};

class Game_Server : public NET_ROLE_SERVER(Game_Shared)
{
    
};

class Game_Client : public NET_ROLE_CLIENT(Game_Shared)
{
};