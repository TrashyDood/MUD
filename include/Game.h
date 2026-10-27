#pragma once

#include "Networking/ClientServerCommon.h"
#include "Networking/Server.h"
#include "Networking/Client.h"

NET_ROLE_ARGS
class Game_Shared : public NET_ROLE_SHARED
{
    NET_TYPES

protected:
    NET_PROP(clientProp, int, NetAuthority::Client) = 42;
    NET_FUNC(serverFunc, int(int, int), NetAuthority::Server) = [this](int a, int b)
    {
        return a + b;
    };
};

class Game_Server : public NET_ROLE_SERVER(Game_Shared)
{
};

class Game_Client : public NET_ROLE_CLIENT(Game_Shared)
{
    NET_OVERRIDE(clientProp) = 42;
    NET_OVERRIDE(serverFunc) = [this](int a, int b)
    {
        return a / b;
    };
};