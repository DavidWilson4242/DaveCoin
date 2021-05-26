#include <iostream>
#include <cryptopp/dsa.h>
#include "blockchain.hpp"
#include "sig.hpp"

int main(int argc, char *argv[]) {
   
  DSA::PublicKey public_key;
  DSA::PrivateKey private_key;

  Sig::LoadKey("keys/public_key.key", public_key);
  Sig::LoadKey("keys/private_key.key", private_key);
  
  std::vector<Tx_Input> inputs;
  std::vector<Tx_Output> outputs;  

  inputs.push_back(Tx_Input());
  outputs.push_back(Tx_Output());

  Tx t0 = Tx::ConstructTransaction(inputs, outputs, public_key, private_key);
  Tx t1 = Tx::Decode(t0.Serialize());

  std::cout << t0.Verify() << std::endl;
  std::cout << (t0.origin == t1.origin) << std::endl;

  return 0;

}
