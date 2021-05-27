#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <string>
#include <map>
#include "server.hpp"
#include "jumbopacket.hpp"

/*
 * Uses a simple TCP library (see ./tcp)
 *
 */

std::map<std::string, Client> connected_clients;
std::map<std::string, bool> client_heartbeats;
  
/* every 5 seconds, send a message to each client.
 * if unsuccessful, removes client from connected list */
void start_heartbeat_loop(TcpServer& server) {
  
  while (true) {
    sleep(NodeServer::KEEP_ALIVE_TIME);
    
    for (auto it = connected_clients.cbegin(); it != connected_clients.cend();) {

    //for (auto &pair: connected_clients) {
      const std::string& IP = it->first;
      const Client& client = it->second;
      
      /* no heartbeat in the time interval? kill connection */ 
      if (client_heartbeats.find(IP) == client_heartbeats.end()) {
	//std::cout << "server: killed connection from client " << IP << std::endl;
	connected_clients.erase(IP);
      }
    } 

    client_heartbeats.clear();

  }

}

void NodeServer::Init() {

  TcpServer server;
  server_observer_t observer;
  
  pipe_ret_t pipe = server.start(NodeServer::PORT);
  if (!pipe.success) {
    throw std::runtime_error("Server setup failed.\n");
  }

  /* configure and register observer
   * wantedIP="" means we're listening from any IP.
   */ 
  observer.incoming_packet_func = NodeServer::ReceiveMessage;
  observer.disconnected_func = NodeServer::ClientDisconnected;
  observer.wantedIp = "";
  server.subscribe(observer);

  std::thread heartbeatLoop(start_heartbeat_loop, std::ref(server));
  
  while (true) {
    Client client = server.acceptClient(0);
    if (client.isConnected()) {
      std::cout << "new client connected with ip " << client.getIp() << std::endl;
    } else {
      throw std::runtime_error("Server failed to accept clients");
    }

    connected_clients[client.getIp()] = client;
    
    std::string message = JumboPacket::SerializeSimpleString("message from server!");
    server.sendToAllClients(message.c_str(), message.size());
    sleep(1);
  } 

  heartbeatLoop.join();

}

void NodeServer::ReceiveMessage(const Client& client, const char *message, size_t size) {
  
  std::string decodedMessage = JumboPacket::DecodePacket(std::string(message, size));
  
  if (decodedMessage == "HEARTBEAT") {
    client_heartbeats[client.getIp()] = true; 
  }

}

void NodeServer::ClientDisconnected(const Client& client) {
  
  std::cout << "poopermanz\n";
  connected_clients.erase(client.getIp());

}
