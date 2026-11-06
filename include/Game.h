#pragma once

#include "Networking/ClientServerCommon.h"
#include "Networking/Server.h"
#include "Networking/Client.h"

NET_ROLE_ARGS
class Game_Shared : public NET_ROLE_SHARED
{
    NET_TYPES

protected:
};

class Game_Server : public NET_ROLE_SERVER(Game_Shared)
{
};

class Game_Client : public NET_ROLE_CLIENT(Game_Shared)
{

};