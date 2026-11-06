#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <type_traits>
#include <vector>

#define WRITE(x) std::cout << x
#define WRITE_LINE(x) WRITE(x) << "\n"
#define WRITE_ERROR(x) std::cerr << x << "\n"

namespace Networking
{
    using Byte = char;

    template <typename TBase, typename TDerived>
    struct AssertInherits
    {
        static_assert(std::is_base_of_v<TBase, TDerived>,
                      "Type must inherit from the required base");
        using type = TDerived;
    };

    namespace PacketsIO
    {
        template <typename T>
        inline constexpr bool ValidType() { return std::is_trivially_copyable_v<T>; }

        template <typename... T>
        inline constexpr bool ValidTypes() { return (ValidType<T> && ...); }

        template <typename T>
        bool TryReadBytes(const std::vector<Byte> &bytes, const size_t index, T &outValue)
        {
            if constexpr (!ValidType<T>())
                return false;

            if (index > bytes.size() || sizeof(T) > bytes.size() - index)
                return false;

            std::memcpy(&outValue, bytes.data() + index, sizeof(T));
            return true;
        }

        template <typename... TValues>
        bool TryReadBytesPackage(const std::vector<Byte> &bytes, const size_t index, TValues &...outValues)
        {
            if constexpr (!ValidTypes<TValues...>())
                return false;

            constexpr size_t packageSize = (size_t{0} + ... + sizeof(TValues));
            if (index > bytes.size() || packageSize > bytes.size() - index)
                return false;

            size_t offset = index;
            const auto readValue = [&bytes, &offset](auto &value)
            {
                if (!TryReadBytes(bytes, offset, value))
                    return false;
                offset += sizeof(value);
                return true;
            };

            bool succeeded = true;
            ((succeeded = succeeded && readValue(outValues)), ...);
            return succeeded;
        }

        template <typename T>
        bool TryWriteBytes(std::vector<Byte> &bytes, const T &value, const size_t index)
        {
            if constexpr (!ValidType<T>())
                return false;

            if (index > bytes.size() || sizeof(T) > bytes.size() - index)
                return false;
            std::memcpy(bytes.data() + index, &value, sizeof(T));
            return true;
        }

        template <typename... TValues>
        bool TryWriteBytesPackage(std::vector<Byte> &bytes, const size_t index, const TValues &...values)
        {
            if constexpr (!ValidTypes<TValues...>())
                return false;

            constexpr size_t packageSize = (size_t{0} + ... + sizeof(TValues));
            if (index > bytes.size() || packageSize > bytes.size() - index)
                return false;

            size_t offset = index;
            const auto writeValue = [&bytes, &offset](const auto &value)
            {
                if (!TryWriteBytes(bytes, value, offset))
                    return false;
                offset += sizeof(value);
                return true;
            };

            bool succeeded = true;
            ((succeeded = succeeded && writeValue(values)), ...);
            if (!succeeded)
                return false;

            return true;
        }
    }

    namespace BitUtils
    {
        template <typename T>
        bool GetBit(T value, unsigned int index)
        {
            static_assert(std::is_integral_v<T> && !std::is_same_v<T, bool>);

            if (index >= sizeof(T) * 8)
                return false;

            using U = std::make_unsigned_t<T>;
            return (static_cast<U>(value) >> index) & 1;
        }

        template <typename T>
        void SetBit(T &value, unsigned int index, bool bit)
        {
            static_assert(std::is_integral_v<T> && !std::is_same_v<T, bool>);

            if (index >= sizeof(T) * 8)
                return;

            using U = std::make_unsigned_t<T>;
            U mask = U{1} << index;
            U result = static_cast<U>(value);

            if (bit)
                result |= mask;
            else
                result &= ~mask;

            value = static_cast<T>(result);
        }

        template <typename T>
        uint8_t GetByte(T value, unsigned int index)
        {
            static_assert(std::is_integral_v<T> && !std::is_same_v<T, bool>);

            if (index >= sizeof(T))
                return 0;

            using U = std::make_unsigned_t<T>;
            return (static_cast<U>(value) >> (index * 8)) & 0xFF;
        }

        template <typename T>
        void SetByte(T &value, unsigned int index, uint8_t byte)
        {
            static_assert(std::is_integral_v<T> && !std::is_same_v<T, bool>);

            if (index >= sizeof(T))
                return;

            using U = std::make_unsigned_t<T>;
            const unsigned int shift = index * 8;
            const U mask = U{0xFF} << shift;
            U result = static_cast<U>(value);
            result = (result & ~mask) | (U{byte} << shift);
            value = static_cast<T>(result);
        }
    }
}
