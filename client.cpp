#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fstream>
#include "client.hpp"

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

/* attempts to connect to a server */
void initialize_client_connection(const std::string& serverIP) {

  TcpClient client;
  
  client_observer_t observer;
  observer.wantedIp = "";
  observer.incoming_packet_func = NodeClient::ReceiveMessage;
  observer.disconnected_func = NodeClient::Disconnected;
  client.subscribe(observer);

  pipe_ret_t connection = client.connectTo(serverIP, NodeClient::PORT);
  if (!connection.success) {
    std::cerr << "couldn't connect to server " << serverIP << " listed in peers.dat" << std::endl;
    return;
  }

  /* send messages to server */
  while (true) {
    std::string message = "what's up bro";
    pipe_ret_t ret = client.sendMsg(message.c_str(), message.size());
    if (!ret.success) {
      throw std::runtime_error("Client failed to send message.");
    }
    sleep(1);
  }
}

/* gets the ip address of my local computer
 * used to ensure that we don't try to connect
 * to our own server. */
static std::string getMyIP() {
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

void NodeClient::ReceiveMessage(const char *message, size_t size) {
  std::cout << "client got message: " << message << std::endl;
}

void NodeClient::Disconnected(const pipe_ret_t& pipe) {

}

void NodeClient::Init() {

  std::string myIP = getMyIP();
  
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
