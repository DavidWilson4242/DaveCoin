#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <vector>
#include <cryptopp/dsa.h>
#include "../tcp/include/tcp_client.h"
#include "tx.hpp"

namespace NodeClient {

  struct NClient {
    std::string serverIP;
    TcpClient client;
    client_observer_t observer;
    std::thread *thread; /* todo fix leak */
    bool alive = false;
  };
  
  const uint32_t PORT = 8080;

  void Init();
  void ConnectToServer(const std::string& IP);
  void ReceiveMessage(const char *message, size_t size);
  void Disconnected(const pipe_ret_t& pipe);
  void BroadcastTx(std::vector<Tx_Input>& inputs,
		   std::vector<Tx_Output>& outputs,
		   const DSA::PublicKey& public_key,
		   const DSA::PrivateKey& private_key);

};

#endif
