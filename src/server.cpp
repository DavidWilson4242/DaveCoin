#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <string>
#include <map>
#include "server.hpp"
#include "client.hpp"
#include "jumbopacket.hpp"

using namespace JumboPacket;
using namespace NodeServer;

NServer server;

/* every NodeServer::KEEP_ALIVE_TIME, the server must receive a HEARTBEAT
 * from a client.  if a heartbeat is not received during that interval,
 * the server will stop broadcasting to that client, and the client
 * will have to reconnect.
 *
 * I couldn't find a working event handler to detect when a client immediately
 * disconnects, so this is the solution that I'm going with for now. */
static void start_heartbeat_loop(NodeServer::NServer& server) { 
  
  while (true) {
   
    for (auto i = server.connected_clients.begin(), 
         e = server.connected_clients.end(); i != e;) {
      
      auto it = i++;
      const std::string& IP = it->first;
      Client& client = it->second;
      
      /* no heartbeat in the time interval? kill connection */ 
      if (server.client_heartbeats.find(IP) == server.client_heartbeats.end()) {
        NodeServer::ClientDisconnected(client);
        server.tserver.killClient(client);
        server.connected_clients.erase(IP);
      }
    } 

    server.client_heartbeats.clear();
    sleep(NodeServer::KEEP_ALIVE_TIME);

  }

}

/* is there a client with IP [IP] listening to me? */
bool NodeServer::HasClientWithIP(const std::string& IP) {
  for (auto& pair: server.connected_clients) {
    if (pair.first == IP) {
      return true;
    }
  }
  return false;
}

void NodeServer::Init() {

  server_observer_t observer;
  
  pipe_ret_t pipe = server.tserver.start(NodeServer::PORT);
  if (!pipe.success) {
    throw std::runtime_error("Server setup failed.\n");
  }

  /* configure and register observer
   * wantedIP="" means we're listening from any IP.
   */ 
  observer.incoming_packet_func = NodeServer::ReceiveMessage;
  observer.disconnected_func = NodeServer::ClientDisconnected;
  observer.wantedIp = "";
  server.tserver.subscribe(observer);

  auto start_client_loop = [&]() {
    while (true) {
      Client client = server.tserver.acceptClient(0);
      if (client.isConnected()) {
        std::cout << "new client connected with ip " << client.getIp() << std::endl;
      } else {
        throw std::runtime_error("Server failed to accept clients");
      }

      server.connected_clients[client.getIp()] = client;
      server.client_heartbeats[client.getIp()] = true;
      
      sleep(1);
    } 
  };
  
  /* for testing... mine a random block */
  auto start_mine_loop = [&]() {
    auto [target_public, target_private] = Sig::GenerateKeys();
    Block b;
    b.hash = "0";
    b.prev_hash = "0";
    b.coinbase.target = target_public;
    b.coinbase.coins = 100;
    b.bit_thresh = 1;
    server.miner.MineBlock(b);
  };

  /* start up all of our threads */

  /* HEARTBEAT LOOP -- make sure we get pings from clients */
  std::thread heartbeatLoop(
    start_heartbeat_loop, 
    std::ref(server)
  );

  /* CLIENT LOOP -- listen for messages from clients */
  std::thread clientLoop(start_client_loop);

  /* MINER LOOP -- mine coins! */
  std::thread mineLoop(start_mine_loop);

  heartbeatLoop.join();
  clientLoop.join();
  mineLoop.join();

}

/* sends a message to client(s)
 * the clients to send to are determined by the packet.targets vector */
void NodeServer::SendMessage(const JumboPacket::EncodedPacket& packet) {
  
  const std::string& serial = packet.serial;

  for (auto client: packet.targets) {
    server.tserver.sendToClient(*client, serial.c_str(), serial.size());
  }

}

void NodeServer::SendMessageToAllClients(const JumboPacket::EncodedPacket& packet) {
  server.tserver.sendToAllClients(packet.serial.c_str(), packet.serial.size());  
}

/* broadcasts a block to all clients */
void NodeServer::BroadcastBlock(const Block& block) {

  auto packet = JumboPacket::SerializeMinedBlock(block); 
  NodeServer::SendMessageToAllClients(packet);


}

/* fired when the server receives a message from [client] */
void NodeServer::ReceiveMessage(const Client& client, const char *message, size_t size) {
  
  std::string packet_raw = std::string(message, size);
  JumboPacket::PacketType pt = JumboPacket::ReadHeader(packet_raw); 
  
  switch (pt) {
    case JumboPacket::CLIENT_HEARTBEAT: 
      server.client_heartbeats[client.getIp()] = true; 
      break;

    case JumboPacket::CLIENT_POKE:
      NodeClient::ConnectToServer(client.getIp());
      break;

    case JumboPacket::SIMPLE_STRING: {
      auto packet = JumboPacket::DecodeSimpleString(packet_raw);
      std::cout << "[server rec'd message]: " << packet.data << std::endl;
      break;
    }

    case JumboPacket::BROADCAST_TX: {
      
      break;
    }

    case JumboPacket::CLIENT_NULL:
    default:
      std::cout << "server received invalid message\n";
      break;

  }

}

/* fired when a client disconnects from my server (this always occurs after
 * the client times out, i.e. they don't send a HEARTBEAT in time */
void NodeServer::ClientDisconnected(const Client& client) {
  std::cout << "server: killed connection from client " << client.getIp() << std::endl;
}
