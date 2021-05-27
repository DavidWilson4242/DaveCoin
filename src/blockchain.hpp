#ifndef BLOCKCHAIN_HPP
#define BLOCKCHAIN_HPP

#include <inttypes.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <memory>
#include <map>
#include <cryptopp/cryptlib.h>
#include <cryptopp/sha.h>
#include "tx.hpp"

struct Mempool {
  
  std::vector<Tx> trans;

};

struct Block {
  
  std::vector<Tx> trans;
  
  uint64_t index;       /* block index */
  uint8_t solved;       /* solved status of this block */ 
  uint8_t bit_thresh;   /* how many leading 0s are needed to solve */
  CryptoPP::SHA256 prev_hash;
  CryptoPP::SHA256 hash;
  uint64_t timestamp;
  uint32_t nonce;
  Tx_Coinbase coinbase;
  
  /* functions */
  std::string Serialize();
  bool IsSolved();
  void Mine();
  CryptoPP::SHA256 &Hash();
  Block();

};

struct Blockchain {
  
  std::shared_ptr<Block> genesis;
  std::map<std::string, std::shared_ptr<Block>> blocks_by_hash;
  Mempool mempool;

  Blockchain();
  void AddBlock(Block&);
  void AcceptBlock(Block&);
  bool IsValidBlock(Block *, Block *);
  bool IsValidChain();

};

#endif
