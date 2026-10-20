#pragma once

#include <iostream>
#include <cstdint>
#include <vector>

#include "PacketsIO.h"
#include "ThreadPool.h"
#include "Utils.h"
#include "TickLoop.h"
#include "NetworkedValues.h"

using Port = unsigned short;
using NetID = short;
using NetPriority = unsigned short;

template <typename TReturnType>
class NetRPC;
template <typename T>
class NetProp;

class NetReceiverSender
{
    inline static NetReceiverSender *instance = nullptr;

protected:
    NetReceiverSender()
    {
        instance = this;
    }

public:
    static NetReceiverSender *GetInstance() { return instance; }

    ~NetReceiverSender()
    {
        if (instance == this)
            instance = nullptr;
    }
};

template <typename T>
class NetRegistry
{
public:
    inline static NetRegistry<T> &Instance()
    {
        static NetRegistry instance;
        return instance;
    }

    bool RegisterProperty(NetProp<T> &netProperty, NetID &outID)
    {
        return true;
    }

    bool UnregisterProperty(NetProp<T> &netProperty)
    {
        return true;
    }
};

template <typename T>
class NetProp
{
    T value;
    NetID netID = -1;

public:
    constexpr bool IsRegistered() const { return netID >= 0; }

    NetProp(T initialValue, NetPriority priority = 0)
        : value(initialValue)
    {
        (void)priority;
    }

    ~NetProp()
    {
        if (!IsRegistered()) return;
    }

    const T &Get() const { return value; }
    void Set(const T &newValue)
    {
        if (newValue == value)
            return;

        // mark for net sync
        value = newValue;
    }

    operator const T &() const { return Get(); }
    NetProp &operator=(const T &newValue)
    {
        Set(newValue);
        return *this;
    }
};

template <typename TReturnType>
class NetRPC
{
public:
    NetRPC() = default;
    ~NetRPC() = default;
};