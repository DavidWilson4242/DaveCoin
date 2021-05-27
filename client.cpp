#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fstream>
#include <map>
#include "client.hpp"
#include "jumbopacket.hpp"

/*
 * peers.dat is a list of IP addresses of nodes in the network.
 * when a node goes online, it broadcasts to each IP on this list
 * to make its prescence known.  the nodes that we reached out to
 * will also connect to OUR server
 *
 * remember: each node is both a client AND server, as this is
 * a peer-to-peer network.  when we go online, it's our job
 * to reach out to the other nodes and connect to their
 * servers.  it's their job to reach back out to us
 * and connect to our server.
 *
 */

/* map of servers that I'm connected to
 * key is the IP address of the server */
std::map<std::string, NodeClient::NodeClient *> connected_servers;

/* attempts to connect to a server */
void initialize_client_connection(const std::string& serverIP) {
  
  NodeClient::NodeClient *nc = new NodeClient::NodeClient;

  nc->observer.wantedIp = "";
  nc->observer.incoming_packet_func = NodeClient::ReceiveMessage;
  nc->observer.disconnected_func = NodeClient::Disconnected;
  nc->client.subscribe(nc->observer);

  pipe_ret_t connection = nc->client.connectTo(serverIP, NodeClient::PORT);
  if (!connection.success) {
    std::cerr << "couldn't connect to server " << serverIP << " listed in peers.dat" << std::endl;
    delete nc;
    return;
  }
  std::cout << "connected to server " << serverIP << " listed in peers.dat" << std::endl;

  /* log in the connections hashtable */
  connected_servers[serverIP] = nc;
  
  nc->alive = true;
  
  /*
   * a server must receive a heartbeat message from each client every
   * KEEP_ALIVE_TIME (see server.h).  If it does not, it will stop
   * listening to that client until it reconnects.  to be safe, we
   * will send a heartbeat every floor(KEEP_ALIVE_TIME/2) seconds.
   *  
   */
  auto keep_alive = [&]() {
    while (nc->alive) {
    
      std::string message = JumboPacket::SerializeHeartbeat();
      pipe_ret_t ret = nc->client.sendMsg(message.c_str(), message.size());
      if (!ret.success) {
	nc->alive = false;
	break;
      }
      sleep(1);

    }
  };

  std::thread heartbeat(keep_alive);

  /* send messages to server */
  while (nc->alive) {
    
    std::string message = JumboPacket::SerializeSimpleString("hello, server!"); 
    pipe_ret_t ret = nc->client.sendMsg(message.c_str(), message.size());
    if (!ret.success) {
      std::cout << "failed to send message to server " << serverIP << ".. aborting.\n";
      nc->alive = false;
      break;
    }
    sleep(1);
  }

  heartbeat.join();
}


void NodeClient::ReceiveMessage(const char *message, size_t size) {
  
  std::string decodedMessage = JumboPacket::DecodePacket(std::string(message, size));

  std::cout << "client got message: " << decodedMessage << std::endl;
}

void NodeClient::Disconnected(const pipe_ret_t& pipe) {
  std::cout << "disconnected\n";
}

void NodeClient::Init() {

  std::string myIP = JumboPacket::GetMyIP();
  
  /* peers.dat contains a list of IP addresses of other nodes.
   * when our node goes online, we attempt to connect to each
   * of these nodes */ 
  std::ifstream peers;
  
  peers.open("peers.dat", std::ios::in);
  if (!peers.is_open()) {
    throw std::runtime_error("NodeClient: failed to open peers.dat");
  }
  
  std::string IP;
  std::vector<std::thread *> client_threads;
  while (std::getline(peers, IP)) {

    /* don't try to connect to myself... */
    if (myIP == IP) {
      std::cout << "tried to connect to myself\n";
      continue;
    } 

    std::thread *client_thread = new std::thread(initialize_client_connection, IP);
    client_threads.push_back(client_thread);

  } 

  /* wait for all threads to complete */
  for (const auto thread: client_threads) {
    thread->join();
    delete thread;
  } 

  peers.close();
}
