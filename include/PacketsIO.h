#pragma once

#include <array>
#include <string>
#include <cstdint>
#include <functional>
#include <limits>

using NetPacket64 = uint64_t; // Contains the data of a packet being sent.
using NetPacketType = uint8_t; // Represents the type of packet being sent.
using PacketBitSize = uint8_t; // Represents the size of a packet.
using PacketPayloadWriteFunc = std::function<bool(const void*, PacketBitSize&, NetPacket64&)>; // Represents a function that writes data to a packet.
using PacketPayloadReadFunc = std::function<void(NetPacket64)>; // Represents a function that reads data from a packet.

constexpr uint8_t NetPacketTypeByteSize() { return sizeof(NetPacketType); }
constexpr uint8_t NetPacketTypeBitSize() { return sizeof(NetPacketType) * 8; }
constexpr size_t NetPacketTypeCount() { return size_t{1} << NetPacketTypeBitSize(); } // Calculates how many packet types there are.

namespace PacketsIO
{
    inline std::array<PacketPayloadWriteFunc, NetPacketTypeCount()> PacketPayloadWriters;
    inline std::array<PacketPayloadReadFunc, NetPacketTypeCount()> PacketPayloadReaders;

    template<typename TPacketPayload>
    bool BuildComplexPacket(NetPacketType packetType, TPacketPayload payload, uint8_t& outPacketBitSize, NetPacket64& outNetPacket)
    {
        // Get the registered writer for this packet type.
        auto& writer = PacketPayloadWriters[packetType];

        if (!writer) return false;

        // Serialize the payload and retrieve its bit size.
        PacketBitSize payloadBitSize = 0;
        NetPacket64 serializedPayload = 0;

        if (!writer(&payload, payloadBitSize, serializedPayload)) return false;

        // Ensure the serialized payload fits within the packet.
        if (payloadBitSize > 64 - NetPacketTypeBitSize()) return false;

        // Combine the serialized payload and packet type.
        outNetPacket = (serializedPayload << NetPacketTypeBitSize()) | packetType;

        // Calculate the total number of bits used.
        outPacketBitSize = NetPacketTypeBitSize() + payloadBitSize;

        return true;
    }

    template<typename TPacketPayload>
    void BuildSimplePacket(NetPacketType packetType, TPacketPayload payload, NetPacket64& outNetPacket)
    {
        outNetPacket = (static_cast<NetPacket64>(payload) << NetPacketTypeBitSize()) | packetType;
    }
}

class BitStream
{
public:
    uint64_t data = 0;
    uint8_t bitPosition = 0;

    uint8_t GetBitSize() const { return bitPosition; }
    uint8_t GetByteSize() const { return (bitPosition + 7) / 8; }

    void ResetPosition() { bitPosition = 0; }

    void Clear()
    {
        data = 0;
        bitPosition = 0;
    }

    template<typename T>
    bool Write(T value)
    {
        static_assert(std::is_integral_v<T> && !std::is_same_v<T, bool>);

        const uint8_t bitCount = sizeof(T) * 8;

        if (bitPosition > 64 - bitCount)
            return false;

        using U = std::make_unsigned_t<T>;

        uint64_t mask = bitCount == 64 ? UINT64_MAX : (uint64_t{1} << bitCount) - 1;
        uint64_t bits = static_cast<uint64_t>(static_cast<U>(value)) & mask;

        data &= ~(mask << bitPosition);
        data |= bits << bitPosition;

        bitPosition += bitCount;

        return true;
    }

    template<typename T>
    bool Read(T& value)
    {
        static_assert(std::is_integral_v<T> && !std::is_same_v<T, bool>);

        const uint8_t bitCount = sizeof(T) * 8;

        if (bitPosition > 64 - bitCount)
            return false;

        uint64_t mask = bitCount == 64 ? UINT64_MAX : (uint64_t{1} << bitCount) - 1;
        uint64_t bits = (data >> bitPosition) & mask;

        value = static_cast<T>(bits);
        bitPosition += bitCount;

        return true;
    }
};