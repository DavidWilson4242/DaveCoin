#ifndef SERVER_HPP
#define SERVER_HPP

#include "tcp/include/tcp_server.h"

namespace NodeServer {
  
  const uint32_t PORT = 97352;
  
  void ReceiveMessage(const Client& client, const char *message, size_t size);
  void ClientDisconnected(const Client& client);
  void Init();

};

#endif
