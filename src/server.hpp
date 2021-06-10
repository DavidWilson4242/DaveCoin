#ifndef SERVER_HPP
#define SERVER_HPP

#include <queue>
#include "tcp/include/tcp_server.h"
#include "blockchain.hpp"
#include "mine.hpp"
#include "jumbopacket.hpp"

/* singleton */

namespace NodeServer {

  const uint32_t PORT = 8080;
  const uint32_t KEEP_ALIVE_TIME = 10;
 
  struct NServer {
    TcpServer tserver;
    Miner miner;
    
    /* messages that are queued to be sent to clients.  to send
     * a message to a client, add it to this queue */
    std::queue<JumboPacket::Packet> messages;

    /* map of clients connected to this server.  key is client IP */
    std::map<std::string, Client> connected_clients;

    std::map<std::string, bool> client_heartbeats;

  };

  bool HasClientWithIP(const std::string& IP); 
  void ReceiveMessage(const Client& client, const char *message, size_t size);
  void ClientDisconnected(const Client& client);
  void BroadcastBlock(const Block& block);
  void StartMining();
  void AddMessageToQueue(const JumboPacket::Packet& packet);

  void SendMessage(const JumboPacket::EncodedPacket& packet);
  void SendMessageToAllClients(const JumboPacket::EncodedPacket& packet);
  
  void Init();

};

#endif
