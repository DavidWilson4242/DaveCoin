#ifndef JUMBOPACKET_HPP
#define JUMBOPACKET_HPP

#include <string>
#include <sstream>
#include <cinttypes>
#include "blockchain.hpp"

/*
 *  every message sent from node to node is done through the JumboPacket::Packet
 *  struct.  a message will be serialized into a string of bytes, and the receiver
 *  will use the packet headers to decode the bytes into a meaningful object 
 *  (... or group of objecs, depending on the message).
 *
 *  see docs/jumbopacket.txt for more info about JumboPacket
 */

namespace JumboPacket {
  
  typedef uint16_t PacketType;

  const uint32_t MAGIC_WORD = 0xFA00AF00;
  
  const int HIGHEST_MESSAGE_TYPE    = 0x0005;
  const PacketType CLIENT_NULL      = 0x0000;
  const PacketType CLIENT_POKE      = 0x0001;
  const PacketType CLIENT_HEARTBEAT = 0x0002;
  const PacketType SIMPLE_STRING    = 0x0003;
  const PacketType MINED_BLOCK      = 0x0004;
  const PacketType BLOCK_HEADERS    = 0x0005;
  
  const int MESSAGE_TYPE_START = 0x04;
  const int DATA_START = 0x06;
  
  struct Packet {

    std::stringstream data;
    uint16_t messageType;
  
    std::string Serialize();

    Packet(uint16_t message_type) {
      data.write(reinterpret_cast<const char *>(&JumboPacket::MAGIC_WORD), sizeof(uint32_t));
      data.write(reinterpret_cast<const char *>(&message_type), sizeof(uint16_t));
    };

  };
  
  template<typename T>
  struct DecodedPacket {
    
    uint16_t messageType;
    T data;

    bool Is(uint16_t _m) { return _m == messageType; }

    DecodedPacket<T>(uint16_t _m, const T& _d) : messageType(_m), data(_d) {}

  };

  std::string GetMyIP();
  PacketType ReadHeader(const std::string& packet);
  
  template<typename T>
  DecodedPacket<T> DecodePacket(const std::string& packet);
  
  /* mw: CLIENT_POKE_WORD */ 
  DecodedPacket<std::string> DecodeClientPoke(const std::string& packet);
  std::string                SerializeClientPoke(const std::string& IP);

  /* mw: CLIENT_HEART_BEAT_WORD */
  DecodedPacket<std::string> DecodeHeartbeat(const std::string& packet);
  std::string                SerializeHeartbeat();

  /* mw: SIMPLE_STRING */
  DecodedPacket<std::string> DecodeSimpleString(const std::string& packet);
  std::string                SerializeSimpleString(const std::string& str);

  /* mw: MINED_BLOCK */
  DecodedPacket<Block>       DecodeMinedBlock(const std::string& packet);
  std::string                SerializeMinedBlock(const Block& block);

};

#endif
