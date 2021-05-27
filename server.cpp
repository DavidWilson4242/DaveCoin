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
void start_heartbeat_loop(TcpServer& server, 
			  std::map<std::string, Client>& connected_clients, 
			  std::map<std::string, bool>& client_heartbeats) {
  
  while (true) {
   
    for (auto i = connected_clients.begin(), e = connected_clients.end(); i != e;) {
      
      auto it = i++;
      const std::string& IP = it->first;
      Client& client = it->second;
      
      /* no heartbeat in the time interval? kill connection */ 
      if (client_heartbeats.find(IP) == client_heartbeats.end()) {
	NodeServer::ClientDisconnected(client);
	server.killClient(client);
	connected_clients.erase(IP);
      }
    } 

    client_heartbeats.clear();
    sleep(NodeServer::KEEP_ALIVE_TIME);

  }

}

bool NodeServer::HasClientWithIP(const std::string& IP) {
  for (auto& pair: connected_clients) {
    if (pair.first == IP) {
      return true;
    }
  }
  return false;
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

  auto start_client_loop = [&]() {
    while (true) {
      Client client = server.acceptClient(0);
      if (client.isConnected()) {
	std::cout << "new client connected with ip " << client.getIp() << std::endl;
      } else {
	throw std::runtime_error("Server failed to accept clients");
      }

      connected_clients[client.getIp()] = client;
      client_heartbeats[client.getIp()] = true;
      
      std::string message = JumboPacket::SerializeSimpleString("message from server!");
      server.sendToAllClients(message.c_str(), message.size());
      sleep(1);
    } 
  };

  std::thread heartbeatLoop(start_heartbeat_loop, 
			    std::ref(server),
			    std::ref(connected_clients),
			    std::ref(client_heartbeats));
  std::thread clientLoop(start_client_loop);

  heartbeatLoop.join();
  clientLoop.join();

}

void NodeServer::ReceiveMessage(const Client& client, const char *message, size_t size) {
  
  JumboPacket::DecodedPacket packd = JumboPacket::DecodePacket(std::string(message, size));
  
  if (packd.Is(JumboPacket::CLIENT_HEARTBEAT)) {
    client_heartbeats[client.getIp()] = true; 
  }

}

void NodeServer::ClientDisconnected(const Client& client) {
  
  std::cout << "server: killed connection from client " << client.getIp() << std::endl;

}
