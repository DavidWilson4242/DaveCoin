#ifndef TX_HPP
#define TX_HPP

#include <cryptopp/sha.h>
#include <cryptopp/dsa.h>
#include <string>
#include <cinttypes>
#include <vector>
#include "sig.hpp"

struct Tx_Input {

  /* the hash of the block that contains the output we're referencing */
  std::string block_hash;

  /* the hash of the transaction we're referencing */
  std::string input_hash;
  
  /* the index of the output we're referencing */
  uint32_t index;

  std::string Serialize() const;

};

struct Tx_Output {
  
  /* public key of our target */
  DSA::PublicKey target; 
  uint32_t index;
  uint64_t coins;

  std::string Serialize() const;

};

struct Tx_Coinbase {

  uint64_t coins;

};

struct Tx {

  uint32_t version;
  uint32_t timestamp;
  std::string hash;
  DSA::PublicKey origin;    /* sender's public key */ 
  std::string sig;
  std::vector<Tx_Input> inputs;
  std::vector<Tx_Output> outputs;

  std::string Serialize() const;
  bool Verify() const;

  static Tx Decode(const std::string&);  
  static Tx ConstructTransaction(std::vector<Tx_Input>& inputs,
				 std::vector<Tx_Output>& outputs,
				 const DSA::PublicKey& public_key,
				 const DSA::PrivateKey& private_key);

};

#endif
