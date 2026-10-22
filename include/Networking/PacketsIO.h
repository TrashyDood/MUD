#pragma once

#include <array>
#include <cstddef>
#include <string>
#include <cstdint>
#include <functional>
#include <limits>
#include <type_traits>
#include <iostream>
#include <cstring>
#include <stdexcept>

using Byte = unsigned char;

class IByteBuffer
{
protected:
    virtual Byte *Data() noexcept = 0;
    virtual const Byte *Data() const noexcept = 0;
    virtual size_t Size() const noexcept = 0;

public:
    virtual ~IByteBuffer() = default;

    template <size_t TBufferSize>
    void SetBytes(const std::array<Byte, TBufferSize> &bytes)
    {
        if (TBufferSize != Size())
            throw std::invalid_argument("ByteBuffer source array size must match buffer size");

        if constexpr (TBufferSize > 0)
            std::memcpy(Data(), bytes.data(), TBufferSize);
    }

    template <typename T>
    T Read(size_t index = 0) const
    {
        static_assert(std::is_trivially_copyable_v<T>, "ByteBuffer::Read requires a trivially copyable type");

        const size_t offset = index;
        if (offset > Size() || sizeof(T) > Size() - offset)
            throw std::out_of_range("ByteBuffer read exceeds buffer size");

        T value;
        std::memcpy(&value, Data() + offset, sizeof(T));
        return value;
    }

    template <typename T>
    void Write(const T &value, size_t &outOffset, const size_t index = 0)
    {
        static_assert(std::is_trivially_copyable_v<T>, "ByteBuffer::Write requires a trivially copyable type");

        const size_t offset = index;
        if (offset > Size() || sizeof(T) > Size() - offset)
            throw std::out_of_range("ByteBuffer write exceeds buffer size");

        std::memcpy(Data() + offset, &value, sizeof(T));
        outOffset = index + sizeof(T);
    }
};

template <size_t TBufferSize>
struct ByteBuffer : IByteBuffer
{
    std::array<Byte, TBufferSize> bytes{};

    const size_t byteCount = TBufferSize;
    const size_t bitCount = TBufferSize * 8;

    ByteBuffer() = default;
    explicit ByteBuffer(std::array<Byte, TBufferSize> bytes) : bytes(bytes) {}

    Byte *Data() noexcept override { return bytes.data(); }
    const Byte *Data() const noexcept override { return bytes.data(); }
    size_t Size() const noexcept override { return bytes.size(); }

    Byte &operator[](size_t index) noexcept { return bytes[index]; }
    const Byte &operator[](size_t index) const noexcept { return bytes[index]; }
};

using ByteBuffer16 = ByteBuffer<2>;
using ByteBuffer24 = ByteBuffer<3>;
using ByteBuffer32 = ByteBuffer<4>;
using ByteBuffer48 = ByteBuffer<6>;
using ByteBuffer64 = ByteBuffer<8>;
using ByteBuffer128 = ByteBuffer<16>;

class ByteBufferIO
{
};

// OBSOLETE
namespace FixedByteBuffer
{
    using NetPacket64 = uint64_t;                                                                     // Contains the data of a packet being sent.
    using NetPacketType = uint8_t;                                                                    // Represents the type of packet being sent.
    using PacketBitSize = uint8_t;                                                                    // Represents the size of a packet.
    using PacketPayloadWriteFunc = std::function<bool(const void *, PacketBitSize &, NetPacket64 &)>; // Represents a function that writes data to a packet.
    using PacketPayloadReadFunc = std::function<void(NetPacket64)>;                                   // Represents a function that reads data from a packet.

    constexpr uint8_t NetPacketTypeByteSize() { return sizeof(NetPacketType); }
    constexpr uint8_t NetPacketTypeBitSize() { return sizeof(NetPacketType) * 8; }
    constexpr size_t NetPacketTypeCount() { return size_t{1} << NetPacketTypeBitSize(); } // Calculates how many packet types there are.

    namespace PacketsIO
    {
        inline std::array<PacketPayloadWriteFunc, NetPacketTypeCount()> PacketPayloadWriters;
        inline std::array<PacketPayloadReadFunc, NetPacketTypeCount()> PacketPayloadReaders;

        template <typename TPacketPayload>
        bool BuildComplexPacket(NetPacketType packetType, TPacketPayload payload, uint8_t &outPacketBitSize, NetPacket64 &outNetPacket)
        {
            // Get the registered writer for this packet type.
            auto &writer = PacketPayloadWriters[packetType];

            if (!writer)
                return false;

            // Serialize the payload and retrieve its bit size.
            PacketBitSize payloadBitSize = 0;
            NetPacket64 serializedPayload = 0;

            if (!writer(&payload, payloadBitSize, serializedPayload))
                return false;

            // Ensure the serialized payload fits within the packet.
            if (payloadBitSize > 64 - NetPacketTypeBitSize())
                return false;

            // Combine the serialized payload and packet type.
            outNetPacket = (serializedPayload << NetPacketTypeBitSize()) | packetType;

            // Calculate the total number of bits used.
            outPacketBitSize = NetPacketTypeBitSize() + payloadBitSize;

            return true;
        }

        template <typename TPacketPayload>
        void BuildSimplePacket(NetPacketType packetType, TPacketPayload payload, NetPacket64 &outNetPacket)
        {
            outNetPacket = (static_cast<NetPacket64>(payload) << NetPacketTypeBitSize()) | packetType;
        }
    }

    template <typename TBufferData = uint64_t>
    class FixedBitBuffer
    {
        static_assert(std::is_integral_v<TBufferData> && !std::is_same_v<TBufferData, bool>);
        static_assert(std::is_unsigned_v<TBufferData>, "FixedBitBuffer requires an unsigned integral buffer type.");

        TBufferData data = 0;
        size_t bitCursor = 0;
        size_t bitSize = 0;
        bool isReading = false;

    public:
        static constexpr TBufferData MaxRange = std::numeric_limits<TBufferData>::max();
        static constexpr uint8_t ByteCapacity() { return sizeof(TBufferData); }
        static constexpr uint8_t BitCapacity() { return ByteCapacity() * 8; }

        FixedBitBuffer() = default;
        explicit FixedBitBuffer(TBufferData initWith) { WriteNext(initWith); }

        size_t GetBitSize() const { return bitSize; }
        size_t GetByteSize() const { return (bitSize + 7) / 8; }
        size_t GetCursor() const { return bitCursor; }
        TBufferData GetData() const { return data; }

        void ResetCursor() { bitCursor = 0; }
        void ClearData()
        {
            data = 0;
            bitSize = 0;
            bitCursor = 0;
            isReading = false;
        }

    private:
        static constexpr TBufferData MakeMask(const uint8_t bitCount)
        {
            if (bitCount == 0)
                return 0;
            if (bitCount >= BitCapacity())
                return MaxRange;
            return (TBufferData{1} << bitCount) - 1;
        }

    public:
        template <typename TValueType>
        bool WriteNext(const TValueType &value, const uint8_t bitCount = static_cast<uint8_t>(sizeof(TValueType) * 8))
        {
            static_assert(std::is_integral_v<TValueType> && !std::is_same_v<TValueType, bool>);

            if (bitCount > BitCapacity())
                return false;

            if (isReading)
            {
                isReading = false;
                ResetCursor();
            }

            if (bitCount == 0)
                return true;
            if (bitCursor > static_cast<size_t>(BitCapacity()) - bitCount)
                return false;

            using U = std::make_unsigned_t<TValueType>;

            const TBufferData mask = MakeMask(bitCount);
            const TBufferData bits = static_cast<TBufferData>(static_cast<U>(value)) & mask;

            data &= ~(mask << bitCursor);
            data |= bits << bitCursor;

            bitCursor += bitCount;
            if (bitCursor > bitSize)
                bitSize = bitCursor;

            return true;
        }

        template <typename TValueType>
        bool ReadNext(TValueType &outValue, const uint8_t bitCount = static_cast<uint8_t>(sizeof(TValueType) * 8))
        {
            static_assert(std::is_integral_v<TValueType> && !std::is_same_v<TValueType, bool>);

            if (!isReading)
            {
                isReading = true;
                ResetCursor();
            }

            if (bitCount == 0)
                return true;
            if (bitCount > bitSize || bitCursor > bitSize - bitCount)
                return false;

            const TBufferData mask = MakeMask(bitCount);
            const TBufferData bits = (data >> bitCursor) & mask;

            outValue = static_cast<TValueType>(bits);
            bitCursor += bitCount;

            return true;
        }
    };

    using FixedBitBuffer16 = FixedBitBuffer<uint16_t>;
    using FixedBitBuffer32 = FixedBitBuffer<uint32_t>;
    using FixedBitBuffer64 = FixedBitBuffer<uint64_t>;
}