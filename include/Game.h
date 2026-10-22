#pragma once

#include "ClientServerCommon.h"

extern NetRegistry ClientNetRegistry;
extern NetRegistry ServerNetRegistry;

template<NetRegistry& TNetRegistry>
class GameBase
{
protected:
    NetRegistry &registry;

public:
    template <typename T>
    using NetProp = NetTypes::NetProp<TNetRegistry, T>;
    template <typename TSignature>
    using NetFunc = NetTypes::NetFunc<TNetRegistry, TSignature>;

    explicit GameBase(NetRegistry &registry);
    virtual ~GameBase();
};

class Game_Client
{
public:
    Game_Client() = default;
};

class Game_Server
{
public:
    Game_Server() = default;
};