#ifndef SERVER_HPP
#define SERVER_HPP

#include "../tcp/include/tcp_server.h"
#include "blockchain.hpp"

namespace NodeServer {
  
  const uint32_t PORT = 8080;
  const uint32_t KEEP_ALIVE_TIME = 10;
  
  bool HasClientWithIP(const std::string& IP); 
  void ReceiveMessage(const Client& client, const char *message, size_t size);
  void ClientDisconnected(const Client& client);
  void BroadcastBlock(const Block& block);
  void Init();

};

#endif
