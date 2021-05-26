#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <string>
#include "server.hpp"

/*
 * Uses a simple TCP library (see ./tcp)
 *
 */

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
  
  while (true) {
    Client client = server.acceptClient(0);
    if (client.isConnected()) {
      server.printClients();
    } else {
      throw std::runtime_error("Server failed to accept clients");
    }
    sleep(1);
  } 

}

void NodeServer::ReceiveMessage(const Client& client, const char *message, size_t size) {
  std::cout << "server got message: " << message << std::endl; 
}

void NodeServer::ClientDisconnected(const Client& client) {

}
