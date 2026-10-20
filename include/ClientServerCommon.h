#pragma once

#include <iostream>
#include <cstdint>

#include "PacketsIO.h"
#include "ThreadPool.h"
#include "Utils.h"
#include "TickLoop.h"

using Port = uint16_t;

class NetReceiverSender
{
    static NetReceiverSender *instance;

protected:
    NetReceiverSender()
    {
        instance = this;
    }

public:
    static NetReceiverSender *GetInstance() { return instance; }

    ~NetReceiverSender()
    {
        instance = nullptr;
    }
};

template <typename TValueType>
class NetProp
{
    TValueType value;

public:
    NetProp(uint16_t priority = 0)
    {
        // register
    }

    ~NetProp()
    {
        // unregister
    }

    const TValueType &Get() const { return value; }
    void Set(const TValueType &newValue)
    {

        value = newValue;
    }

    operator const TValueType &() const { return Get(); }
    NetProp &operator=(const TValueType &newValue) { Set(newValue); return *this; }
};

template <typename TReturnType>
class NetRPC
{
public:
    NetRPC()
    {
        // register
    }

    ~NetRPC()
    {
        // unregister
    }
};