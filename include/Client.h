#pragma once

#include "ClientServerCommon.h"

class Client : public NetReceiverSender
{
    Client() = default;

public:
    static Client* CreateClient();

    int Connect();
    int Disconnect();
};