#include <iostream>
#include "client.hpp"

void NodeClient::ReceiveMessage(const char *message, size_t size) {

}

void NodeClient::Disconnected(const pipe_ret_t& pipe) {

}

void NodeClient::Init() {

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
    std::string message = "what's up fucker";
    pipe_ret_t ret = client.sendMsg(message.c_str(), message.size());
    if (!ret.success) {
      throw std::runtime_error("Client failed to send message.");
    }
    sleep(1);
  }
}
