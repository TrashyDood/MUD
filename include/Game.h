#pragma once

#include "Networking/ClientServerCommon.h"
#include "Networking/Server.h"
#include "Networking/Client.h"

template <Networking::NetRegistry &TNetRegistry, bool TIsServer>
class GameNetData
{
public:
    NET_DATA_TYPES(TNetRegistry)

protected:
    NetFunc<void(int, int)> testfunc;
    NetProp<int> test = 0;
};

extern Networking::NetRegistry ClientNetRegistry;
class Game_Client : public Networking::Client, public GameNetData<ClientNetRegistry, false>
{
public:
    Game_Client()
    {
        testfunc = [this](int, int) {};
    }
};

extern Networking::NetRegistry ServerNetRegistry;
class Game_Server : public Networking::Server, public GameNetData<ServerNetRegistry, true>
{
public:
    Game_Server() = default;
};