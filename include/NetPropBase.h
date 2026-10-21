#pragma once

#include "NetRegistry.h"

#include <utility>

class NetObj
{
    friend class NetRegistry;

    NetId id = -1;
    NetRegistry *registry = nullptr;

public:
    NetObj(const NetObj &) = delete;
    NetObj &operator=(const NetObj &) = delete;
    NetObj(NetObj &&) = delete;
    NetObj &operator=(NetObj &&) = delete;

    NetId GetNetID() const { return id; }
    bool IsRegistered() const { return id >= 0; }
    NetRegistry *GetRegistry() const { return registry; }

    void SetRegistry(NetRegistry *registry);

    NetObj(NetRegistry &registry);
    virtual ~NetObj();
};

template <typename T>
class NetProp : public NetObj
{
    T value, pendingValue;
    NetPriority priority;

    void ApplyPending()
    {
        if (!IsDirty())
            return;

        value = pendingValue;
    }

public:
    const T &Get() const { return value; }
    const T &GetPending() const { return pendingValue; }
    constexpr bool IsDirty() const { return value != pendingValue; }

    NetProp(T initialValue = T{}, const NetPriority priority = 0, NetRegistry &registry = StaticNetRegistry)
        : NetObj(registry), value(initialValue), pendingValue(std::move(initialValue)), priority(priority) {}

    bool SetPending(const T &newValue)
    {
        if (newValue == pendingValue)
            return false;

        // mark for net sync
        pendingValue = newValue;
        return true;
    }

    operator const T &() const { return GetPending(); }
    NetProp &operator=(const T &pendingValue)
    {
        SetPending(pendingValue);
        return *this;
    }
};