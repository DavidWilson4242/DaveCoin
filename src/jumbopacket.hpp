#ifndef JUMBOPACKET_HPP
#define JUMBOPACKET_HPP

#include <string>
#include <sstream>
#include <cinttypes>

/*
 *  every message sent from node to node is done through the JumboPacket::Packet
 *  struct.  a message will be serialized into a string of bytes, and the receiver
 *  will use the packet headers to decode the bytes into a meaningful object 
 *  (... or group of objecs, depending on the message).
 *
 *  see docs/jumbopacket.txt for more info about JumboPacket
 */

namespace JumboPacket {
  
  const uint32_t MAGIC_WORD = 0xFA00AF00;

  const uint16_t CLIENT_NULL = 0x0000;
  const uint16_t CLIENT_POKE = 0x0001;
  const uint16_t CLIENT_HEARTBEAT = 0x0002;
  const uint16_t SIMPLE_STRING = 0x0003;
  const uint16_t FULL_BLOCK = 0x0004;
  const uint16_t BLOCK_HEADERS = 0x0005;
  
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

  struct DecodedPacket {
    
    uint16_t messageType;
    std::string data;

    bool Is(uint16_t _m) { return _m == messageType; }

    DecodedPacket(uint16_t _m, const std::string& _d) : messageType(_m), data(_d) {}

  };

  std::string GetMyIP();

  DecodedPacket DecodePacket(const std::string& packet);
  
  /* mw: CLIENT_POKE_WORD */ 
  std::string   SerializeClientPoke(const std::string& IP);
  DecodedPacket DecodeClientPoke(const std::string& packet);

  /* mw: CLIENT_HEART_BEAT_WORD */
  std::string   SerializeHeartbeat();
  DecodedPacket DecodeHeartbeat(const std::string& packet);

  /* mw: SIMPLE_STRING */
  std::string   SerializeSimpleString(const std::string& str);
  DecodedPacket DecodeSimpleString(const std::string& packet);

};

#endif
