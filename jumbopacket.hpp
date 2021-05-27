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
 *  the packet headers are as follows:
 *
 *  [0, 4)  -> MAGIC_WORD
 *  [4, 6)  -> messageType (CLIENT_POKE_WORD, ...)
 *  [6, 10) -> messageLength (does NOT include MAGIC_WORD ot messageType)
 *  [10, ...] -> messageData
 *
 */

namespace JumboPacket {
  
  const uint32_t MAGIC_WORD = 0xFFAABBCC;
  const uint16_t CLIENT_POKE = 0x0001;
  const uint16_t CLIENT_HEARTBEAT = 0x0002;
  const uint16_t CLIENT_SIMPLE_STRING = 0x0003;
  
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

  std::string GetMyIP();

  std::string DecodePacket(const std::string& packet);
  
  /* mw: CLIENT_POKE_WORD */ 
  std::string SerializeClientPoke(const std::string& IP);
  std::string DecodeClientPoke(const std::string& packet);

  /* mw: CLIENT_HEART_BEAT_WORD */
  std::string SerializeHeartbeat();
  std::string DecodeHeartbeat(const std::string& packet);

  /* mw: CLIENT_SIMPLE_STRING */
  std::string SerializeSimpleString(const std::string& str);
  std::string DecodeSimpleString(const std::string& packet);

};

#endif
