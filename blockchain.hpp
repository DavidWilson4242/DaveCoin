#ifndef BLOCKCHAIN_HPP
#define BLOCKCHAIN_HPP

#include <inttypes.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <memory>
#include <map>
#include "crypto.hpp"

/* a coin vector is split into units, whole units and millionth units */
struct CoinVector {

  uint64_t unit;
  uint64_t subunit;

  CoinVector(uint64_t u, uint64_t su) : unit(u), subunit(su) {}
  CoinVector() : unit(0), subunit(0) {}

};

struct Coinbase {
  
  std::string rec;
  CoinVector reward;

};

/* a transaction input is actually the output of a previous trade */
struct TransactionInput {
  
  /* points to the hash of the block the transaction containing
     the output we're looking for */
  std::vector<uint8_t> block_hash;

  /* points to the hash of the transaction within the block */
  std::vector<uint8_t> input_hash;

  /* the index inside of the transaction */
  uint32_t index;
  

};

struct TransactionOutput {

};

struct Transaction {

  uint32_t version;
  std::vector<TransactionInput> inputs;
  std::vector<TransactionOutput> outputs;
  uint32_t timestamp;

  Transaction();

};

struct Block {
  
  std::vector<Transaction> trans;
  
  uint64_t index;       /* block index */
  uint8_t solved;       /* solved status of this block */ 
  uint8_t bit_thresh;   /* how many leading 0s are needed to solve */
  std::vector<uint8_t> prev_hash;
  std::vector<uint8_t> hash;
  uint64_t timestamp;
  uint32_t nonce;
  Coinbase coinbase;
  
  /* functions */
  bool IsSolved();
  void Mine();
  std::vector<uint8_t> &Hash();
  Block();

};

struct Blockchain {
  
  std::shared_ptr<Block> genesis;
  std::map<std::string, std::shared_ptr<Block>> blocks_by_hash;

  Blockchain();
  void AddBlock(Block&);
  void AcceptBlock(Block&);
  bool IsValidBlock(Block *, Block *);
  bool IsValidChain();

};

#endif
