#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fstream>
#include <map>
#include <cmath>
#include "client.hpp"
#include "server.hpp"
#include "jumbopacket.hpp"

using namespace JumboPacket;

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

std::vector<NodeClient::NClient *> active_clients;

/* attempts to connect to a server */
void initialize_client_connection(const std::string& serverIP, NodeClient::NClient *nc) {
  
  nc->serverIP = serverIP;
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

  active_clients.push_back(nc);

  nc->alive = true;
  
  /*
   * a server must receive a heartbeat message from each client every
   * KEEP_ALIVE_TIME (see server.h).  If it does not, it will stop
   * listening to that client until it reconnects.  to be safe, we
   * will send a heartbeat every floor(KEEP_ALIVE_TIME/3) seconds.
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
      sleep(ceil((float)NodeServer::KEEP_ALIVE_TIME/3.0));
    }
  };
  std::thread heartbeat(keep_alive);
  
  /* when I connect to a server, I give them a *poke* with a message containing
   * my server's IP address.  if they are not connected to my server, they
   * will try to connect */
  if (!NodeServer::HasClientWithIP(serverIP)) {
    std::string message = JumboPacket::SerializeClientPoke(JumboPacket::GetMyIP());
    nc->client.sendMsg(message.c_str(), message.size());
  }

  /* send messages to server */
  while (nc->alive) {
    std::string message = "hello from client " + JumboPacket::GetMyIP();
    std::string serial = JumboPacket::SerializeSimpleString(message); 
    pipe_ret_t ret = nc->client.sendMsg(serial.c_str(), serial.size());
    if (!ret.success) {
      std::cout << "failed to send message to server " << serverIP << ".. aborting.\n";
      nc->alive = false;
      break;
    }
    sleep(1);
  }

  heartbeat.join();
}

/* this function is called when the server I'm listening to sends me a message */
/* see jumbopacket.hpp and docs/jumbopacket.txt for documentation on message formats */
void NodeClient::ReceiveMessage(const char *message, size_t size) {
  
  std::string packet_raw = std::string(message, size);

  JumboPacket::PacketType pt = JumboPacket::ReadHeader(packet_raw);

  switch (pt) {
    case JumboPacket::SIMPLE_STRING: {
      DecodedPacket<std::string> packet = DecodeSimpleString(packet_raw);
      std::cout << "[client rec'd message]: " << packet.data << std::endl;
      break;
    }

    case JumboPacket::CLIENT_NULL:
    default:
      std::cerr << "client received invalid message\n";
      break;

  }

}

void NodeClient::Disconnected(const pipe_ret_t& pipe) {
  std::cout << "my client disconnected from a server\n";
}

void NodeClient::ConnectToServer(const std::string& IP) {
  /* don't try to connect to myself... */
  if (JumboPacket::GetMyIP() == IP) {
    return;
  } 

  /* don't connect a second time */
  for (const auto ec: active_clients) {
    if (ec->serverIP == IP) {
      return;
    }
  }

  /* try to connect */
  NodeClient::NClient *nc = new NodeClient::NClient();
  nc->thread = new std::thread(initialize_client_connection, IP, nc);
}

void NodeClient::BroadcastTx(std::vector<Tx_Input>& inputs,
                             std::vector<Tx_Output>& outputs,
			     const DSA::PublicKey& public_key,
			     const DSA::PrivateKey& private_key) {
  

}

void NodeClient::Init() {

  /* peers.dat contains a list of IP addresses of other nodes.
   * when our node goes online, we attempt to connect to each
   * of these nodes */ 
  std::ifstream peers;
  
  peers.open("dat/peers.dat", std::ios::in);
  if (!peers.is_open()) {
    throw std::runtime_error("NodeClient: failed to open peers.dat");
  }
  
  std::string IP;
  while (std::getline(peers, IP)) {
    ConnectToServer(IP);
  } 
  peers.close();

  /* main client loop */
  while (active_clients.size() > 0) {
    active_clients[0]->thread->join(); 
    delete active_clients[0]->thread;
    delete active_clients[0];
    active_clients.erase(active_clients.begin());

  }

}
