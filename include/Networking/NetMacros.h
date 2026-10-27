#pragma once  

#define INVALID_NET_ID -1

// Gets a locally defined net registry for a class.
#define GET_NET_REGISTRY _net_registry_

// Declare local net types for a class, which will be used for all net objects in that class.
#define NET_TYPES(isServer)                                                                                  \
    using NetRegistry = Networking::NetRegistry;                                                             \
    using NetAuthority = Networking::NetAuthority;                                                           \
    template <typename T, NetAuthority Authority = NetAuthority::Shared>                                     \
    using NetProp = Networking::NetProp<T, Authority, isServer>;                                             \
    template <typename TSignature, NetAuthority Authority = NetAuthority::Shared, bool TReassignable = true> \
    using NetFunc = Networking::NetFunc<TSignature, TReassignable, Authority, isServer>;

// Declare a local net registry for a class, which will be used for all net objects in that class.
#define NET_REGISTRY(...) \
    Networking::NetRegistry GET_NET_REGISTRY { __VA_ARGS__ }

// Declare a local net registry reference for a scope, which will be used for all net objects in that class.
#define NET_REGISTRY_REF \
    Networking::NetRegistry &GET_NET_REGISTRY

// Declare a local net property for a class.
#define NET_PROP(name, ...) \
    NetProp<__VA_ARGS__> name = Networking::NetObjectInitializer<NetProp<__VA_ARGS__>> { this->GET_NET_REGISTRY }

// Declare a scope.local net class.
#define NET_FUNC(name, ...) \
    NetFunc<__VA_ARGS__> name = Networking::NetObjectInitializer<NetFunc<__VA_ARGS__>> { this->GET_NET_REGISTRY }

// Override the value of a net property or function.
#define NET_OVERRIDE(target) [[maybe_unused]] decltype(target) &_net_override_##target##_ = (target)

#define NET_ROLE_SHARED(isServer) std::conditional_t<isServer, Networking::Server, Networking::Client>
#define NET_ROLE_CLIENT(netRoleShared) Utils::AssertInherits<NET_ROLE_SHARED(false), netRoleShared<false>>::type
#define NET_ROLE_SERVER(netRoleShared) Utils::AssertInherits<NET_ROLE_SHARED(true), netRoleShared<true>>::type