#pragma once

#define NET_REGISTRY_NAME _net_registry_
#define NET_OVERRIDE_NAME(name) _net_override_##name##_

// Gets a locally defined net registry for a class.
#define GET_NET_REGISTRY(...) __VA_OPT__((__VA_ARGS__)->) NET_REGISTRY_NAME

// Declare local net types for a class, which will be used for all net objects in that class.
#define NET_TYPES(isServer)                                                                                  \
    using NetRegistry = Networking::NetRegistry;                                                             \
    using NetAuthority = Networking::NetAuthority;                                                           \
    template <typename T, NetAuthority Authority = NetAuthority::Shared>                                     \
    using NetProp = Networking::NetProp<T, Authority, isServer>;                                             \
    template <typename TSignature, NetAuthority Authority = NetAuthority::Shared, bool TReassignable = true> \
    using NetFunc = Networking::NetFunc<TSignature, TReassignable, Authority, isServer>;

// Declare local net type aliases for a class, which will be used for all net objects in that class.
#define NET_ROLE_TYPES(netRole)                                                                              \
    using NetAuthority = Networking::NetAuthority;                                                           \
    template <typename T, NetAuthority Authority = NetAuthority::Shared>                                     \
    using NetProp = typename netRole::template NetProp<T, Authority>;                                        \
    template <typename TSignature, NetAuthority Authority = NetAuthority::Shared, bool TReassignable = true> \
    using NetFunc = typename netRole::template NetFunc<TSignature, Authority, TReassignable>;

// Declare a local net registry for a class, which will be used for all net objects in that class.
#define NET_REGISTRY(...) \
    Networking::NetRegistry NET_REGISTRY_NAME { __VA_ARGS__ }

// Declare a local net registry reference for a class, which will be used for all net objects in that class.
#define NET_REGISTRY_REF \
    Networking::NetRegistry &NET_REGISTRY_NAME

// Declare a local net property for a class.
#define NET_PROP(name, ...) \
    NetProp<__VA_ARGS__> name = Networking::NetObjectInitializer<NetProp<__VA_ARGS__>> { GET_NET_REGISTRY(this) }

// Declare a local net function for a class.
#define NET_FUNC(name, ...) \
    NetFunc<__VA_ARGS__> name = Networking::NetObjectInitializer<NetFunc<__VA_ARGS__>> { GET_NET_REGISTRY(this) }

// Override the value of a net property or function.
#define NET_OVERRIDE(target) [[maybe_unused]] decltype(target) &NET_OVERRIDE_NAME(target) = (target)

#define NET_ROLE_SHARED(isServer) std::conditional_t<isServer, Networking::Server, Networking::Client>
#define NET_ROLE_CLIENT(netRoleShared) Utils::AssertInherits<NET_ROLE_SHARED(false), netRoleShared<false>>::type
#define NET_ROLE_SERVER(netRoleShared) Utils::AssertInherits<NET_ROLE_SHARED(true), netRoleShared<true>>::type