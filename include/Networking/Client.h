#pragma once

#include "ClientServerCommon.h"

namespace Networking
{
    class Client
    {
    protected:
        Client() = default;

    public:
        int Connect();
        int Disconnect();
    };
}