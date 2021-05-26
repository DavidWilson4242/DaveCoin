#include <iostream>
#include <cryptopp/dsa.h>
#include "blockchain.hpp"
#include "sig.hpp"

int main(int argc, char *argv[]) {
   
  // my keys
  DSA::PublicKey public_key;
  DSA::PrivateKey private_key;
  Sig::LoadKey("keys/public_key.key", public_key);
  Sig::LoadKey("keys/private_key.key", private_key);

  // target keys
  auto [target_public, target_private] = Sig::GenerateKeys();
  
  // build transaction 
  std::vector<Tx_Input> inputs;
  std::vector<Tx_Output> outputs;  

  Tx_Input input;
  Tx_Output output;

  input.block_hash = "ABCDEFG";
  input.input_hash = "XYZZZZZ";
  input.index = 3;

  output.target = public_key;
  output.index = 3;
  output.coins = 100;

  inputs.push_back(input);
  outputs.push_back(output);

  Tx t0 = Tx::ConstructAndSign(inputs, outputs, public_key, private_key);
  Tx t1 = Tx::DecodeAndVerify(t0.Serialize(), t0.sig, public_key); 
  
  std::cout << (t1.outputs[0].target == t0.outputs[0].target) << std::endl;

  return 0;

}
