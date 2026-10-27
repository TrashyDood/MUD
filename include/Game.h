#pragma once

#include "Networking/ClientServerCommon.h"
#include "Networking/Server.h"
#include "Networking/Client.h"

template <bool TIsServer>
class Game_Shared : public NET_ROLE_SHARED(TIsServer)
{
    NET_TYPES(TIsServer)
    
protected:
};

class Game_Server : public NET_ROLE_SERVER(Game_Shared)
{
};

class Game_Client : public NET_ROLE_CLIENT(Game_Shared)
{
};