#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "client.hpp"

/* gets the ip address of my local computer
 * used to ensure that we don't try to connect
 * to our own server. */
static std::string getMyIP() {
  int sock = socket(PF_INET, SOCK_DGRAM, 0);
  sockaddr_in loopback;

  if (sock == -1) {
    throw std::runtime_error("getMyIp: could not socket");
  }

  std::memset(&loopback, 0, sizeof(loopback));
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

}

void NodeClient::Disconnected(const pipe_ret_t& pipe) {

}

void NodeClient::Init() {

  getMyIP();

  TcpClient client;
  
  client_observer_t observer;
  observer.wantedIp = "127.0.0.1";
  observer.incoming_packet_func = NodeClient::ReceiveMessage;
  observer.disconnected_func = NodeClient::Disconnected;
  client.subscribe(observer);

  pipe_ret_t connection = client.connectTo("127.0.0.1", NodeClient::PORT);
  if (!connection.success) {
    throw std::runtime_error("Client failed to connect.");
  } 

  /* send messages to server */
  while (true) {
    std::string message = "hello, server!";
    pipe_ret_t ret = client.sendMsg(message.c_str(), message.size());
    if (!ret.success) {
      throw std::runtime_error("Client failed to send message.");
    }
    sleep(1);
  }
}
