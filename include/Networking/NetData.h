#pragma once

#include <functional>
#include <utility>
#include <cstddef>
#include <queue>
#include <type_traits>
#include <vector>

#define CLIENT_SERVER_RESTRICTS

namespace Networking
{
    using NetId = int;
    using NetPriority = short;

    class NetObj;

    enum class RPCAuthority : char
    {
        ServerAndClient = 0,
        ClientOnly = 1,
        ServerOnly = 2
    };

    class NetRegistry
    {
        friend class NetObj;

        std::vector<NetObj *> netObjects;
        std::queue<NetId> vacantIds;

        void Register(NetObj &netObject);
        void Unregister(NetObj &netObject);

    public:
        NetRegistry() = default;
        NetObj *Find(NetId id) const;
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

    template <NetRegistry &TRegistry, typename T, bool TCanSend = true, bool TCanReceive = true>
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

    template <NetRegistry &TRegistry, typename TSignature, bool TReassignable = true, bool TCanSend = true, bool TCanReceive = true>
    class NetFunc;

    template <NetRegistry &TRegistry, typename TReturn, typename... TArgs, bool TReassignable, bool TCanSend, bool TCanReceive>
    class NetFunc<TRegistry, TReturn(TArgs...), TReassignable, TCanSend, TCanReceive> : public NetObj
    {
        std::function<TReturn(TArgs...)> function;

        TReturn Invoke(TArgs... args) const
        {
            return (function) ? function(std::forward<TArgs>(args)...) : TReturn{};
        }

    public:
        explicit NetFunc(std::function<TReturn(TArgs...)> function = nullptr, const NetPriority priority = 0)
            : NetObj(TRegistry, priority), function(std::move(function)) {}

        template <bool TEnabled = TReassignable, std::enable_if_t<!TEnabled, int> = 0>
        NetFunc &operator=(std::function<TReturn(TArgs...)>) = delete;

        template <bool TEnabled = TReassignable, std::enable_if_t<TEnabled, int> = 0>
        NetFunc &operator=(std::function<TReturn(TArgs...)> newFunction) { function = std::move(newFunction); return *this; }

        template <bool TEnabled = TReassignable, std::enable_if_t<!TEnabled, int> = 0>
        NetFunc &operator=(std::nullptr_t) = delete;

        template <bool TEnabled = TReassignable, std::enable_if_t<TEnabled, int> = 0>
        NetFunc &operator=(std::nullptr_t) { function = nullptr; return *this; }

        TReturn operator()(TArgs... args) const { return Invoke(std::forward<TArgs>(args)...); }
    };

#define NET_DATA_TYPES(registry, isServer)                               \
    template <typename T, bool TCanSend = true, bool TCanRecieve = true> \
    using NetProp = Networking::NetProp<registry, T>;                    \
    template <typename TSignature, bool TReassignable = true,            \
              bool TCanSend = true, bool TCanRecieve = true>             \
    using NetFunc = Networking::NetFunc<registry, TSignature, TReassignable>;
}