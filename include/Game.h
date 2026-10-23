#pragma once

#include "Networking/ClientServerCommon.h"
#include "Networking/Server.h"
#include "Networking/Client.h"

template <bool TIsServer>
class GameNetState
{
    NET_REGISTRY(TIsServer, Net)

protected:
    NetProp<int, NetAuthority::Client> testProp;
};

class Game_Client : public Networking::Client, public GameNetState<false>
{
public:
    Game_Client()
    {
    }
};

class Game_Server : public Networking::Server, public GameNetState<true>
{
public:
    Game_Server() = default;
};