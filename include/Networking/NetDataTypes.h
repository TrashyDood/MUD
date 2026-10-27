#pragma once

#include <functional>
#include <type_traits>
#include <utility>

#include "NetData.h"

#define OBJ_TEMPLATE_ARGS NetAuthority TAuthority, bool TIsServerSide
#define PROP_TEMPLATE_ARGS typename T, OBJ_TEMPLATE_ARGS
#define FUNC_TEMPLATE_ARGS typename TSignature, bool TReassignable, OBJ_TEMPLATE_ARGS
#define TEMPL_RESTRICT_ARGS(condition) bool TEnabled = true, std::enable_if_t < TEnabled && (condition), int > = 0

#define ASSIGN_OP(type, templateArgs, newValue, deletedCondition, enabledCondition) \
    template <templateArgs, TEMPL_RESTRICT_ARGS((deletedCondition))>                \
    type &operator=(newValue) = delete;                                             \
    template <templateArgs, TEMPL_RESTRICT_ARGS((enabledCondition))>                \
    type &operator=(newValue)

#define CALL_OP(returnType, templateArgs, args, deletedCondition, enabledCondition) \
    template <templateArgs, TEMPL_RESTRICT_ARGS((deletedCondition))>                \
    returnType operator()(args) const = delete;                                     \
    template <templateArgs, TEMPL_RESTRICT_ARGS((enabledCondition))>                \
    returnType operator()(args) const

namespace Networking
{
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

    template <OBJ_TEMPLATE_ARGS>
    class NetObj : public NetObjBase
    {
    public:
        bool ServerSide() const override { return TIsServerSide; }
        bool CanSend() const override { return NetCanSend<TAuthority, TIsServerSide>; }
        bool CanReceive() const override { return NetCanReceive<TAuthority, TIsServerSide>; }

        explicit NetObj(NetRegistry &registry, const NetPriority priority = 0)
            : NetObjBase(registry, priority) {}
    };

    template <PROP_TEMPLATE_ARGS>
    class NetProp : public NetObj<TAuthority, TIsServerSide>
    {
        using Base = NetObj<TAuthority, TIsServerSide>;

        T value, pendingValue;

    public:
        const T &GetValue() const { return value; }
        const T &GetPendingValue() const { return pendingValue; }

        NetProp(NetRegistry &registry, T initialValue = T{}, const NetPriority priority = 0)
            : Base(registry, priority), value(initialValue), pendingValue(std::move(initialValue)) {}

        operator const T &() const { return GetPendingValue(); }

        ASSIGN_OP(NetProp, typename TUnused = void, const T &newValue, (!NetCanSend<TAuthority, TIsServerSide>), (NetCanSend<TAuthority, TIsServerSide>))
        {
            if (newValue != pendingValue)
            {
                this->MarkPending();
                pendingValue = newValue;
            }
            return *this;
        }
    };

    template <FUNC_TEMPLATE_ARGS>
    class NetFunc : public NetObj<TAuthority, TIsServerSide>
    {
        using ReturnType = typename std::function<TSignature>::result_type;
        using Base = NetObj<TAuthority, TIsServerSide>;
        using Func = std::function<TSignature>;

        Func function;

    public:
        static constexpr bool IsReassignable = TReassignable && NetCanReceive<TAuthority, TIsServerSide>;

        template <typename TFunction>
        static constexpr bool IsCallableFunction =
            std::is_invocable_r_v<ReturnType, TFunction &> &&
            !std::is_same_v<std::decay_t<TFunction>, Func>;

        template <typename TFunction>
        static constexpr bool CanAssignFunction = IsReassignable && IsCallableFunction<TFunction>;

        NetFunc(NetRegistry &registry, Func initialFunction = nullptr, const NetPriority priority = 0)
            : Base(registry, priority), function(std::move(initialFunction)) {}

        template <typename TFunction, TEMPL_RESTRICT_ARGS((std::is_invocable_r_v<ReturnType, TFunction &>))>
        NetFunc(NetRegistry &registry, TFunction &&initialFunction, const NetPriority priority = 0)
            : Base(registry, priority), function(std::forward<TFunction>(initialFunction)) {}

        CALL_OP(ReturnType, typename... TArgs, TArgs &&...args, (!NetCanSend<TAuthority, TIsServerSide>), (NetCanSend<TAuthority, TIsServerSide>))
        {
            if (function)
            {
                this->MarkPending();
                return function(std::forward<TArgs>(args)...);
            }
            if constexpr (!std::is_void_v<ReturnType>)
                return ReturnType{};
        }

        ASSIGN_OP(NetFunc, typename TUnused = void, Func newFunction, (!IsReassignable), (IsReassignable))
        {
            function = std::move(newFunction);
            return *this;
        }

        ASSIGN_OP(NetFunc, typename TUnused = void, std::nullptr_t, (!IsReassignable), (IsReassignable))
        {
            function = nullptr;
            return *this;
        }

        ASSIGN_OP(NetFunc, typename TFunction, TFunction &&newFunction, (IsCallableFunction<TFunction> && !IsReassignable), (CanAssignFunction<TFunction>))
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
