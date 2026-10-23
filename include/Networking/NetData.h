#pragma once

#include <functional>
#include <utility>
#include <cstddef>
#include <queue>
#include <type_traits>
#include <vector>

namespace Networking
{
    using NetId = int;
    using NetPriority = short;

    class NetObj;

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
        NetRegistry &registry;
        NetPriority priority = 0;

    public:
        NetObj(const NetObj &) = delete;
        NetObj &operator=(const NetObj &) = delete;
        NetObj(NetObj &&) = delete;
        NetObj &operator=(NetObj &&) = delete;

        NetId GetNetID() const { return id; }
        NetPriority GetNetPriority() const { return priority; }
        bool IsRegistered() const { return id >= 0; }
        NetRegistry &GetRegistry() const { return registry; }

        NetObj(NetRegistry &registry, const NetPriority priority = 0);
        virtual ~NetObj();
    };

    template <NetRegistry &TRegistry, typename T, bool TIsServerSide, bool TCanSend = true, bool TCanReceive = true>
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

    template <NetRegistry &TRegistry, typename TSignature, bool TIsServerSide, bool TCanSend = true, bool TCanReceive = true, bool TReassignable = true>
    class NetFunc;

    template <NetRegistry &TRegistry, typename TReturn, typename... TArgs, bool TIsServerSide, bool TCanSend, bool TCanReceive, bool TReassignable>
    class NetFunc<TRegistry, TReturn(TArgs...), TIsServerSide, TCanSend, TCanReceive, TReassignable> : public NetObj
    {
        std::function<TReturn(TArgs...)> function;

        TReturn Invoke(TArgs... args) const
        {
            return (function) ? function(std::forward<TArgs>(args)...) : TReturn{};
        }

    public:
        explicit NetFunc(std::function<TReturn(TArgs...)> function = nullptr, const NetPriority priority = 0)
            : NetObj(TRegistry, priority), function(std::move(function)) {}

        TReturn operator()(TArgs... args) const { return Invoke(std::forward<TArgs>(args)...); }

        template <bool TEnabled = TReassignable, std::enable_if_t<!TEnabled, int> = 0>
        NetFunc &operator=(std::function<TReturn(TArgs...)>) = delete;

        template <bool TEnabled = TReassignable, std::enable_if_t<TEnabled, int> = 0>
        NetFunc &operator=(std::function<TReturn(TArgs...)> newFunction) { function = std::move(newFunction); return *this; }

        template <bool TEnabled = TReassignable, std::enable_if_t<!TEnabled, int> = 0>
        NetFunc &operator=(std::nullptr_t) = delete;

        template <bool TEnabled = TReassignable, std::enable_if_t<TEnabled, int> = 0>
        NetFunc &operator=(std::nullptr_t) { function = nullptr; return *this; }
    };
}

// this is fucking madness but why not
#define NET_REGISTRY_IMPL(isServer)                                                           \
    template <bool TRegistryIsServer>                                                         \
    struct NetRegistryStorage { inline static Networking::NetRegistry registry; };            \
                                                                                              \
    inline static Networking::NetRegistry &registry = NetRegistryStorage<isServer>::registry; \
                                                                                              \
    enum class NetAuthority                                                                   \
    {                                                                                         \
        Server,                                                                               \
        Client,                                                                               \
        Shared,                                                                               \
        Local                                                                                 \
    };                                                                                        \
                                                                                              \
    template <typename T, NetAuthority Authority>                                             \
    using NetProp = Networking::NetProp<                                                      \
        NetRegistryStorage<isServer>::registry,                                               \
        T,                                                                                    \
        isServer,                                                                             \
        (Authority == NetAuthority::Server && isServer) ||                                    \
            (Authority == NetAuthority::Client && !isServer) ||                               \
            (Authority == NetAuthority::Shared),                                              \
        (Authority == NetAuthority::Server && !isServer) ||                                   \
            (Authority == NetAuthority::Client && isServer) ||                                \
            (Authority == NetAuthority::Shared)>;                                             \
                                                                                              \
    template <typename TSignature, NetAuthority Authority, bool TReassignable = true>         \
    using NetFunc = Networking::NetFunc<                                                      \
        NetRegistryStorage<isServer>::registry,                                               \
        TSignature,                                                                           \
        isServer,                                                                             \
        (Authority == NetAuthority::Server && isServer) ||                                    \
            (Authority == NetAuthority::Client && !isServer) ||                               \
            (Authority == NetAuthority::Shared),                                              \
        (Authority == NetAuthority::Server && !isServer) ||                                   \
            (Authority == NetAuthority::Client && isServer) ||                                \
            (Authority == NetAuthority::Shared),                                              \
        TReassignable>;

#define NET_REGISTRY_1(isServer) NET_REGISTRY_IMPL(isServer)
#define NET_REGISTRY_2(isServer, scope)                                               \
    struct scope { NET_REGISTRY_IMPL(isServer) };                                     \
    using NetAuthority = typename scope::NetAuthority;                                \
    template <typename T, NetAuthority Authority>                                     \
    using NetProp = typename scope::template NetProp<T, Authority>;                   \
    template <typename TSignature, NetAuthority Authority, bool TReassignable = true> \
    using NetFunc = typename scope::template NetFunc<TSignature, Authority, TReassignable>;
#define NET_REGISTRY_SELECT(_1, _2, NAME, ...) NAME
#define NET_REGISTRY(...) NET_REGISTRY_SELECT(__VA_ARGS__, NET_REGISTRY_2, NET_REGISTRY_1)(__VA_ARGS__)
