#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include "jumbopacket.hpp"

std::string JumboPacket::Packet::Serialize() {
  return data.str();
}

/*
 * let A_C, A_S represent node A's client and server respectively
 * let B_C, B_S represent node B's client and server respectively
 *
 * say that node A joins the network.  A_C will try to connect to
 * B_S if the IP address of B_S is listed in peers.dat.  once connected,
 * A_C will send the IP address of A_S to B_S, and B_C will connect
 * to A_S.
 *
 * TLDR: it's a handshake.  your client connects to my server, my
 * client will connect to your server */ 
std::string JumboPacket::SerializeClientPoke(const std::string& IP) {
  
  JumboPacket::Packet packet(JumboPacket::CLIENT_POKE);
  uint32_t size = (uint32_t)IP.size();

  packet.data.write(reinterpret_cast<const char *>(&size), sizeof(uint32_t)); 
  packet.data << IP;

  return packet.Serialize();
 
}

/* extracts an IP address from ClientPoke message */
std::string JumboPacket::DecodeClientPoke(const std::string& packet) {
  
  const char *data = packet.c_str();

  uint32_t ipLength = *(uint32_t *)&data[JumboPacket::DATA_START];

  return std::string(&data[JumboPacket::DATA_START + 4], ipLength);

}

std::string JumboPacket::SerializeHeartbeat() {
  
  JumboPacket::Packet packet(JumboPacket::CLIENT_HEARTBEAT);

  return packet.Serialize();

}

std::string JumboPacket::DecodeHeartbeat(const std::string& packet) {

  return "HEARTBEAT";
}

std::string JumboPacket::SerializeSimpleString(const std::string& str) {

  JumboPacket::Packet packet(JumboPacket::CLIENT_SIMPLE_STRING);
  
  uint32_t size = (uint32_t)str.size();

  packet.data.write(reinterpret_cast<const char *>(&size), sizeof(uint32_t)); 
  packet.data << str;

  return packet.Serialize();

}

std::string JumboPacket::DecodeSimpleString(const std::string& packet) {

  const char *data = packet.c_str();

  uint32_t strLength = *(uint32_t *)&data[JumboPacket::DATA_START];

  return std::string(&data[JumboPacket::DATA_START + 4], strLength);

}

std::string JumboPacket::DecodePacket(const std::string& packet) {
  
  const char *data = packet.c_str();

  /* must include at least magic word and type */
  if (packet.size() < 6) {
    throw std::runtime_error("JumboPacket: invalid packet");
  }

  uint32_t magic_word = *(uint32_t *)&data[0];
  uint16_t message_type = *(uint16_t *)&data[4];

  if (magic_word != JumboPacket::MAGIC_WORD) {
    throw std::runtime_error("JumboPacket DecodePacket: invalid magic word");
  }

  switch (message_type) {
    case CLIENT_POKE:
      return JumboPacket::DecodeClientPoke(packet);
    case CLIENT_HEARTBEAT:
      return JumboPacket::DecodeHeartbeat(packet);
    case CLIENT_SIMPLE_STRING:
      return JumboPacket::DecodeSimpleString(packet);
    default:
      throw std::runtime_error("JumboPacket DecodePacket: invalid messageType");
  };

  return "";

}

/* gets the ip address of my local computer
 * used to ensure that we don't try to connect
 * to our own server. */
std::string JumboPacket::GetMyIP() {

  int sock = socket(PF_INET, SOCK_DGRAM, 0);
  sockaddr_in loopback;

  if (sock == -1) {
    throw std::runtime_error("getMyIp: could not socket");
  }

  memset(&loopback, 0, sizeof(loopback));
  loopback.sin_family = AF_INET;
  loopback.sin_addr.s_addr = 1337;   // can be any IP address
  loopback.sin_port = htons(9);      // using debug port

  if (connect(sock, reinterpret_cast<sockaddr*>(&loopback), sizeof(loopback)) == -1) {
    close(sock);
    throw std::runtime_error("getMyIp: could not connect");
  }

  socklen_t addrlen = sizeof(loopback);
  if (getsockname(sock, reinterpret_cast<sockaddr*>(&loopback), &addrlen) == -1) {
    close(sock);
    throw std::runtime_error("getMyIp: could not getsockname");
  }

  close(sock);

  char buf[INET_ADDRSTRLEN];
  if (inet_ntop(AF_INET, &loopback.sin_addr, buf, INET_ADDRSTRLEN) == 0x0) {
    std::cerr << "Could not inet_ntop\n";
  }
  return std::string(buf);

}
