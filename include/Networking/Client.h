#pragma once

#include "ClientServerCommon.h"

namespace Networking
{
    class Client : public NetReceiverSender<false>
    {
    public:
        int Connect();
        int Disconnect();
    };
}