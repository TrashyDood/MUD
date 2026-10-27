#pragma once

#include <functional>
#include <type_traits>
#include <tuple>
#include <utility>

#include "NetData.h"

#define TEMPL_RESTRICT_ARGS(condition) bool TEnabled = true, std::enable_if_t < TEnabled && (condition), int > = 0

#define ASSIGN_OP(owner, newValue, deletedCondition, enabledCondition, ...)      \
    template <__VA_OPT__(__VA_ARGS__, ) TEMPL_RESTRICT_ARGS((deletedCondition))> \
    owner &operator=(newValue) = delete;                                         \
    template <__VA_OPT__(__VA_ARGS__, ) TEMPL_RESTRICT_ARGS((enabledCondition))> \
    owner &operator=(newValue)

#define CALL_OP(returnType, deletedCondition, enabledCondition, ...) \
    template <TEMPL_RESTRICT_ARGS((deletedCondition))>               \
    returnType operator()(__VA_ARGS__) = delete;                     \
    template <TEMPL_RESTRICT_ARGS((enabledCondition))>               \
    returnType operator()(__VA_ARGS__)

namespace Networking
{
    inline constexpr ReceiverId DefaultReceiverId = -1;

    template <NetAuthority Authority, bool TIsServerSide>
    inline constexpr bool NetCanSend =
        (Authority == NetAuthority::Server && TIsServerSide) ||
        (Authority == NetAuthority::Client && !TIsServerSide) ||
        (Authority == NetAuthority::Shared);

    template <NetAuthority Authority, bool TIsServerSide>
    inline constexpr bool NetCanReceive =
        (Authority == NetAuthority::Server && !TIsServerSide) ||
        (Authority == NetAuthority::Client && TIsServerSide) ||
        (Authority == NetAuthority::Shared);

    template <NetAuthority Authority, bool TIsServerSide, typename... TToSend>
    inline constexpr bool NetCanSendPayload =
        NetCanSend<Authority, TIsServerSide> &&
        (sizeof...(TToSend) > 0) &&
        (!std::is_void_v<TToSend> && ...);

    template <NetAuthority TAuthority, bool TIsServerSide, typename... TToSend>
    class NetObj : public NetObjBase
    {
    public:
        using PayloadValue = std::tuple<TToSend...>;

    private:
        PayloadValue lastPendingSend{};

    public:
        bool ServerSide() const override { return TIsServerSide; }
        bool CanSend() const override { return NetCanSend<TAuthority, TIsServerSide>; }
        bool CanReceive() const override { return NetCanReceive<TAuthority, TIsServerSide>; }
        const PayloadValue &GetPendingPayloads() const { return lastPendingSend; }

        explicit NetObj(NetRegistry &registry, const NetPriority priority = 0)
            : NetObjBase(registry, priority) {}

        bool TrySend(const TToSend &...payloads, ReceiverId receiverId = DefaultReceiverId)
        {
            (void)sizeof...(payloads); // Avoid unused parameter warnings.

            PayloadValue payload = std::make_tuple(payloads...);
            if (!NetObjBase::TrySend(receiverId, PendingSend::Create(GetNetID(), payload)))
                return false;

            lastPendingSend = std::move(payload);

            return true;
        }

        CALL_OP(bool, (!NetCanSend<TAuthority, TIsServerSide>), (NetCanSendPayload<TAuthority, TIsServerSide, TToSend...>),
                const TToSend &...payloads, ReceiverId receiverId = DefaultReceiverId) { return TrySend(payloads..., receiverId); }
    };

    template <typename T, NetAuthority TAuthority, bool TIsServerSide, typename TToSend = T>
    class NetProp : public NetObj<TAuthority, TIsServerSide, TToSend>
    {
        using Base = NetObj<TAuthority, TIsServerSide, TToSend>;

        T value, pendingValue;

    public:
        const T &GetValue() const { return value; }
        const T &GetPendingValue() const { return pendingValue; }

        explicit NetProp(NetRegistry &registry, T initialValue = T{}, const NetPriority priority = 0)
            : Base(registry, priority), value(initialValue), pendingValue(std::move(initialValue)) {}

        void SetValue(const T &newValue)
        {
            if (newValue != value)
            {
                Base::TrySend(newValue);
                pendingValue = newValue;
            }
        }

        operator const T &() const { return GetPendingValue(); }

        ASSIGN_OP(NetProp, const T &newValue,
                  (!NetCanSend<TAuthority, TIsServerSide>), (NetCanSend<TAuthority, TIsServerSide>))
        {
            SetValue(newValue);
            return *this;
        }
    };

    template <typename TSignature, bool TReassignable, NetAuthority TAuthority, bool TIsServerSide>
    class NetFunc;

    template <typename TReturnType, typename... TArgs, bool TReassignable, NetAuthority TAuthority, bool TIsServerSide>
    class NetFunc<TReturnType(TArgs...), TReassignable, TAuthority, TIsServerSide> : public NetObj<TAuthority, TIsServerSide, TArgs...>
    {
        using Base = NetObj<TAuthority, TIsServerSide, TArgs...>;
        using Func = std::function<TReturnType(TArgs...)>;

        Func function;

    public:
        static constexpr bool IsReassignable = TReassignable && NetCanReceive<TAuthority, TIsServerSide>;

        template <typename TFunction>
        static constexpr bool IsCallableFunction =
            std::is_invocable_r_v<TReturnType, TFunction &, TArgs...> &&
            !std::is_same_v<std::decay_t<TFunction>, Func>;

        template <typename TFunction>
        static constexpr bool IsAssignableFunction =
            IsReassignable &&
            IsCallableFunction<TFunction>;

        NetFunc(NetRegistry &registry, Func initialFunction = nullptr, const NetPriority priority = 0)
            : Base(registry, priority), function(std::move(initialFunction)) {}

        template <typename TFunction, TEMPL_RESTRICT_ARGS((std::is_invocable_r_v<TReturnType, TFunction &, TArgs...>))>
        NetFunc(NetRegistry &registry, TFunction &&initialFunction, const NetPriority priority = 0)
            : Base(registry, priority), function(std::forward<TFunction>(initialFunction)) {}

        TReturnType Invoke(TArgs... args, ReceiverId receiverId = DefaultReceiverId)
        {
            Base::TrySend(std::forward<TArgs>(args)..., receiverId);

            if (function)
                return function(std::forward<TArgs>(args)...);

            if constexpr (!std::is_void_v<TReturnType>)
                return TReturnType{};
        }

        CALL_OP(TReturnType, (!NetCanSend<TAuthority, TIsServerSide>), (NetCanSendPayload<TAuthority, TIsServerSide, TArgs...>),
                TArgs... args, ReceiverId receiverId = DefaultReceiverId) { return Invoke(std::forward<TArgs>(args)..., receiverId); }

        ASSIGN_OP(NetFunc, Func newFunction, (!IsReassignable), (IsReassignable))
        {
            function = std::move(newFunction);
            return *this;
        }

        ASSIGN_OP(NetFunc, std::nullptr_t, (!IsReassignable), (IsReassignable))
        {
            function = nullptr;
            return *this;
        }

        ASSIGN_OP(NetFunc, TFunction &&newFunction,
                  (IsCallableFunction<TFunction> && !IsReassignable), (IsAssignableFunction<TFunction>),
                  typename TFunction)
        {
            function = std::forward<TFunction>(newFunction);
            return *this;
        }
    };
}

#undef OBJ_TEMPLATE_ARGS
#undef PROP_TEMPLATE_ARGS
#undef FUNC_TEMPLATE_ARGS
#undef TEMPL_RESTRICT_ARGS
#undef ASSIGN_OP
#undef CALL_OP
