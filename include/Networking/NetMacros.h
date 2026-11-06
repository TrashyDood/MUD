#pragma once

#include "NetData.h"

// Gets a locally defined net registry for a class.
#define GET_NET_REGISTRY _net_registry_
#define GET_NET_REGISTRY_STAT Networking::StaticNetRegistry<TIsServer>()

// Declare local net types for a class, which will be used for all net objects in that class.
#define NET_TYPES                                                                                            \
    using NetRegistry = Networking::NetRegistry;                                                             \
    using NetAuthority = Networking::NetAuthority;                                                           \
    template <typename T, NetAuthority Authority = NetAuthority::Shared, typename TToSend = T>               \
    using NetProp = Networking::NetProp<T, Authority, TIsServer, TToSend>;                                   \
    template <typename TSignature, NetAuthority Authority = NetAuthority::Shared, bool TReassignable = true> \
    using NetFunc = Networking::NetFunc<TSignature, TReassignable, Authority, TIsServer>;

// Declare a local net registry for a class, which will be used for all net objects in that class.
#define NET_REGISTRY(...) \
    Networking::NetRegistry GET_NET_REGISTRY { __VA_ARGS__ }

// Declare a local net registry reference for a scope, which will be used for all net objects in that class.
#define NET_REGISTRY_REF \
    Networking::NetRegistry &GET_NET_REGISTRY

// Declare a local net property for a class.
#define NET_PROP(name, ...) \
    NetProp<__VA_ARGS__> name = Networking::NetObjectInitializer<NetProp<__VA_ARGS__>> { *this }

// Declare a scope.local net class.
#define NET_FUNC(name, ...) \
    NetFunc<__VA_ARGS__> name = Networking::NetObjectInitializer<NetFunc<__VA_ARGS__>> { *this }

// Override the value of a net property or function.
#define NET_OVERRIDE(target) [[maybe_unused]] decltype(target) &_net_override_##target##_ = (target)

#define NET_ROLE_ARGS template <bool TIsServer>
#define NET_ROLE_SHARED std::conditional_t<TIsServer, Networking::Server, Networking::Client>
#define NET_ROLE_CLIENT(netRoleShared) Networking::AssertInherits<Networking::Client, netRoleShared<false>>::type
#define NET_ROLE_SERVER(netRoleShared) Networking::AssertInherits<Networking::Server, netRoleShared<true>>::type