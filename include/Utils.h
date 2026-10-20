#pragma once

#include <cstdint>
#include <type_traits>

#define WRITE(x) std::cout << x
#define WRITE_LINE(x) WRITE(x) << "\n"
#define WRITE_ERROR(x) std::cerr << x << "\n"

namespace BitUtils
{
    // Read bit
    template <typename T>
    bool GetBit(T value, unsigned int index)
    {
        static_assert(std::is_integral_v<T> && !std::is_same_v<T, bool>);

        if (index >= sizeof(T) * 8)
            return false;

        using U = std::make_unsigned_t<T>;

        return (static_cast<U>(value) >> index) & 1;
    }

    // Write bit
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

    // Read byte
    template <typename T>
    uint8_t GetByte(T value, unsigned int index)
    {
        static_assert(std::is_integral_v<T> && !std::is_same_v<T, bool>);

        if (index >= sizeof(T))
            return 0;

        using U = std::make_unsigned_t<T>;

        return (static_cast<U>(value) >> (index * 8)) & 0xFF;
    }

    // Write byte
    template <typename T>
    void SetByte(T &value, unsigned int index, uint8_t byte)
    {
        static_assert(std::is_integral_v<T> && !std::is_same_v<T, bool>);

        if (index >= sizeof(T))
            return;

        using U = std::make_unsigned_t<T>;

        unsigned int shift = index * 8;
        U mask = U{0xFF} << shift;
        U result = static_cast<U>(value);

        result = (result & ~mask) | (U{byte} << shift);

        value = static_cast<T>(result);
    }
}