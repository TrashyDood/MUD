#pragma once

#include "NetRegistry.h"

#include <functional>
#include <utility>

enum class RPCCallOrder : char
{
    ServerAndClient = 0,
    ClientOnly = 1,
    ServerOnly = 2
};

class NetObj
{
    friend class NetRegistry;

    NetId id = -1;
    NetRegistry *registry = nullptr;
    NetPriority priority = 0;

    void SetRegistry(NetRegistry *newRegistry);

public:
    NetObj(const NetObj &) = delete;
    NetObj &operator=(const NetObj &) = delete;
    NetObj(NetObj &&) = delete;
    NetObj &operator=(NetObj &&) = delete;

    NetId GetNetID() const { return id; }
    NetPriority GetNetPriority() const { return priority; }
    bool IsRegistered() const { return id >= 0; }
    NetRegistry *GetRegistry() const { return registry; }

    NetObj(NetRegistry &registry, const NetPriority priority = 0);
    virtual ~NetObj();

    void RebindRegistry(NetRegistry &registry) { SetRegistry(&registry); }
};

namespace NetTypes
{
    template <NetRegistry &TRegistry, typename T>
    class NetProp : public NetObj
    {
        T value, pendingValue;

    public:
        const T &Get() const { return value; }
        const T &GetPending() const { return pendingValue; }

        NetProp(T initialValue = T{}, const NetPriority priority = 0)
            : NetObj(TRegistry, priority), value(initialValue), pendingValue(std::move(initialValue)) {}

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

    template <NetRegistry &TRegistry, typename TSignature>
    class NetFunc;

    template <NetRegistry &TRegistry, typename TReturn, typename... TArgs>
    class NetFunc<TRegistry, TReturn(TArgs...)> : public NetObj
    {
        const std::function<TReturn(TArgs...)> function;

    public:
        explicit NetFunc(std::function<TReturn(TArgs...)> function, const NetPriority priority = 0)
            : NetObj(TRegistry, priority), function(std::move(function)) {}

        TReturn operator()(TArgs... args) const { return Invoke(std::forward<TArgs>(args)...); }

        TReturn Invoke(TArgs... args) const
        {
            return function(std::forward<TArgs>(args)...);
        }
    };
}