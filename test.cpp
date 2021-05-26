#include <iostream>
#include <thread>
#include <cryptopp/dsa.h>
#include "blockchain.hpp"
#include "sig.hpp"
#include "server.hpp"
#include "client.hpp"

void test_transaction() {
  // my keys (load from file)
  DSA::PublicKey public_key;
  DSA::PrivateKey private_key;
  Sig::LoadKey("keys/public_key.key", public_key);
  Sig::LoadKey("keys/private_key.key", private_key);

  // target keys, the address we're sending a transaction to
  auto [target_public, target_private] = Sig::GenerateKeys();
  
  // build transaction 
  std::vector<Tx_Input> inputs;
  std::vector<Tx_Output> outputs;  

  Tx_Input input;
  Tx_Output output;
  
  // dummy hashes
  input.block_hash = "ABCDEFG";
  input.input_hash = "XYZZZZZ";
  input.index = 3;

  // transaction target
  output.target = target_public;
  output.index = 3;
  output.coins = 100;

  inputs.push_back(input);
  outputs.push_back(output);
  
  // create transaction and sign it w/ our private key
  Tx t0 = Tx::ConstructAndSign(inputs, outputs, public_key, private_key);

  // decode the transaction (this would actually be done after a
  // node receives word of the transaction via a network call)
  Tx t1 = Tx::DecodeAndVerify(t0.Serialize(), t0.sig, public_key); 
  
  // was it decoded properly? 
  bool success = t1.outputs[0].target == t0.outputs[0].target;
  if (success) {
    std::cout << "successfully decoded tx packet\n";
  } else {
    std::cout << "failed to decode tx packet\n";
  }
}

void test_server() {
  
  std::thread server_thread(Server::Init);
  std::thread client_thread(Client::Init);

  server_thread.join();
  client_thread.join();

}

void test_client() {

}

int main(int argc, char *argv[]) {
   
  test_transaction();
  test_server();
  test_client();

  return 0;

}
