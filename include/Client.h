#pragma once

#include "ClientServerCommon.h"

class Client
{
    Client() = default;

public:
    int Connect();
    int Disconnect();
};