#pragma once

#include <cstdint>
#include <type_traits>

#define WRITE(x) std::cout << x
#define WRITE_LINE(x) WRITE(x) << "\n"
#define WRITE_ERROR(x) std::cerr << x << "\n"

namespace BitUtils
{
    // Read bit
    template <typename TValueType>
    bool GetBit(TValueType value, unsigned int index)
    {
        static_assert(std::is_integral_v<TValueType> && !std::is_same_v<TValueType, bool>);

        if (index >= sizeof(TValueType) * 8)
            return false;

        using U = std::make_unsigned_t<TValueType>;

        return (static_cast<U>(value) >> index) & 1;
    }

    // Write bit
    template <typename TValueType>
    void SetBit(TValueType &value, unsigned int index, bool bit)
    {
        static_assert(std::is_integral_v<TValueType> && !std::is_same_v<TValueType, bool>);

        if (index >= sizeof(TValueType) * 8)
            return;

        using U = std::make_unsigned_t<TValueType>;

        U mask = U{1} << index;
        U result = static_cast<U>(value);

        if (bit)
            result |= mask;
        else
            result &= ~mask;

        value = static_cast<TValueType>(result);
    }

    // Read byte
    template <typename TValueType>
    uint8_t GetByte(TValueType value, unsigned int index)
    {
        static_assert(std::is_integral_v<TValueType> && !std::is_same_v<TValueType, bool>);

        if (index >= sizeof(TValueType))
            return 0;

        using U = std::make_unsigned_t<TValueType>;

        return (static_cast<U>(value) >> (index * 8)) & 0xFF;
    }

    // Write byte
    template <typename TValueType>
    void SetByte(TValueType &value, unsigned int index, uint8_t byte)
    {
        static_assert(std::is_integral_v<TValueType> && !std::is_same_v<TValueType, bool>);

        if (index >= sizeof(TValueType))
            return;

        using U = std::make_unsigned_t<TValueType>;

        unsigned int shift = index * 8;
        U mask = U{0xFF} << shift;
        U result = static_cast<U>(value);

        result = (result & ~mask) | (U{byte} << shift);

        value = static_cast<TValueType>(result);
    }
}