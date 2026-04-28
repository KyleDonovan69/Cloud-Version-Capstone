#include "Network/NetworkProtocol.h"

namespace Network {

PacketWriter::PacketWriter(PacketType type)
{
    m_packet << static_cast<std::uint8_t>(type);
}

void PacketWriter::writeUInt8(std::uint8_t value)
{
    m_packet << value;
}

void PacketWriter::writeUInt16(std::uint16_t value)
{
    m_packet << value;
}

void PacketWriter::writeUInt32(std::uint32_t value)
{
    m_packet << value;
}

void PacketWriter::writeFloat(float value)
{
    m_packet << value;
}

void PacketWriter::writeString(const std::string& value)
{
    m_packet << value;
}

void PacketWriter::writeBool(bool value)
{
    m_packet << static_cast<std::uint8_t>(value ? 1 : 0);
}

PacketReader::PacketReader(sf::Packet& packet)
    : m_packet(packet)
    , m_valid(true)
{
}

bool PacketReader::readUInt8(std::uint8_t& value)
{
    if (!(m_packet >> value))
    {
        m_valid = false;
        return false;
    }
    return true;
}

bool PacketReader::readUInt16(std::uint16_t& value)
{
    if (!(m_packet >> value))
    {
        m_valid = false;
        return false;
    }
    return true;
}

bool PacketReader::readUInt32(std::uint32_t& value)
{
    if (!(m_packet >> value))
    {
        m_valid = false;
        return false;
    }
    return true;
}

bool PacketReader::readFloat(float& value)
{
    if (!(m_packet >> value))
    {
        m_valid = false;
        return false;
    }
    return true;
}

bool PacketReader::readString(std::string& value)
{
    if (!(m_packet >> value))
    {
        m_valid = false;
        return false;
    }
    return true;
}

bool PacketReader::readBool(bool& value)
{
    std::uint8_t byteValue;
    if (!(m_packet >> byteValue))
    {
        m_valid = false;
        return false;
    }
    value = (byteValue != 0);
    return true;
}

}
