#include <iostream>
#include <cryptopp/dsa.h>
#include "blockchain.hpp"
#include "sig.hpp"

int main(int argc, char *argv[]) {
  
  std::string signature;
  std::string message = "Hello, world!";

  DSA::PublicKey public_key;
  DSA::PrivateKey private_key;

  Sig::LoadKey("keys/public_key.key", public_key);
  Sig::LoadKey("keys/private_key.key", private_key);
  
  signature = Sig::SignMessage(message, public_key, private_key);
  
  bool valid = Sig::ValidateSignature(message, signature, public_key);

  std::cout << valid << std::endl;

  return 0;

}
