#include <iostream>
#include <thread>
#include <cryptopp/dsa.h>
#include "blockchain.hpp"
#include "sig.hpp"
#include "server.hpp"
#include "client.hpp"

/* tests building a block, serilizing and decoding */
void test_block(DSA::PublicKey& miner, Tx& tx) {
  
  Block b0;
  b0.tx.push_back(tx);
  b0.hash = "0";
  b0.prev_hash = "0";
  b0.coinbase.target = miner;
  b0.coinbase.coins = 100;
  
  /* ensure that serialization works properly */
  Block b1 = Block::DecodeBlock(b0.Serialize());
  if (b0.Serialize() == b1.Serialize()) {
    std::cout << "successfully encoded/decoded block\n";
  }

}

void test_transactions() {
  // my keys (load from file)
  DSA::PublicKey public_key;
  DSA::PrivateKey private_key;
  Sig::LoadKey("keys/public_key.key", public_key);
  Sig::LoadKey("keys/private_key.key", private_key);

  // target keys, the address we're sending a transaction to
  auto [target_public, target_private] = Sig::GenerateKeys();
  
  // build.txaction 
  std::vector<Tx_Input> inputs;
  std::vector<Tx_Output> outputs;  

  Tx_Input input;
  Tx_Output output;
  
  // dummy hashes
  input.block_hash = "ABCDEFG";
  input.input_hash = "XYZZZZZ";
  input.index = 3;

  //.txaction target
  output.target = target_public;
  output.index = 3;
  output.coins = 100;

  inputs.push_back(input);
  outputs.push_back(output);
  
  // create.txaction and sign it w/ our private key
  Tx t0 = Tx::ConstructAndSign(inputs, outputs, public_key, private_key);

  // decode the.txaction (this would actually be done after a
  // node receives word of the.txaction via a network call)
  Tx t1 = Tx::DecodeAndVerify(t0.Serialize(), t0.sig, public_key); 
  
  // was it decoded properly? 
  bool success = t1.outputs[0].target == t0.outputs[0].target;
  if (success) {
    std::cout << "successfully decoded tx packet\n";
  } else {
    std::cout << "failed to decode tx packet\n";
  }

  test_block(public_key, t0);
}

/*
 *  Each node in the JumboCoin peer-to-peer network is both a server and client 
 *  at the same time.  The server is used to listen for requests from other 
 *  nodes, and the client is used to send messages to other nodes.
 *
 *  std::thread is needed to run both simultaneously
 *
 */
void test_server_client() {
  
  std::thread server_thread(NodeServer::Init);
  std::thread client_thread(NodeClient::Init);

  server_thread.join();
  client_thread.join();

}

int main(int argc, char *argv[]) {
   
  test_transactions();
  test_server_client();

  return 0;

}
