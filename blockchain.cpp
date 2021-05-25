#include <cstring>
#include <ctime>
#include <iostream>
#include <iomanip>
#include <limits>
#include <cryptopp/rsa.h>
#include "blockchain.hpp"

Block::Block() {
  
  index = 0;
  timestamp = time(NULL);
  nonce = 0;
  bit_thresh = 3;
  coinbase.rec = "";
  coinbase.reward.unit = 100;
  coinbase.reward.subunit = 0;
  
  prev_hash.reserve(Crypto::SHA256_BUFFER_SIZE);
  for (int i = 0; i < Crypto::SHA256_BUFFER_SIZE; i++) {
    prev_hash.push_back(0x00);
  }

}

Blockchain::Blockchain() {
  
  /* initialize the genesis block.  this is always the same */
  genesis = std::shared_ptr<Block>(new Block);
  genesis->index = 0;
  genesis->timestamp = time(NULL);
  genesis->nonce = 0;
  genesis->bit_thresh = 3;
  genesis->coinbase.rec = "N/A";

}

void Blockchain::AddBlock(Block& b) {

  b.Hash();

}

Transaction::Transaction() {

}


std::vector<uint8_t>& Block::Hash() {

  hash = std::vector<uint8_t>();  
  return hash;
  
}

bool Block::IsSolved() {
  for (int i = 0; i < bit_thresh; i++) {
    if (hash[i] > 0) {
      return false;
    }
  }
  return true;
}

void Block::Mine() {

  while (true) {

    timestamp = static_cast<uint64_t>(std::time(0));

    for (nonce = 0; nonce < std::numeric_limits<uint32_t>::max(); nonce++) {
      Hash();
      if (IsSolved()) {
	std::cout << "Solved block.  Hash:\n";
	std::cout << Crypto::SHA256_tostring(hash) << std::endl;
	return;
      }
    }

  }

}

bool Blockchain::IsValidBlock(Block *block, Block *prev_block) {
 
  if (block == nullptr) {
    return false;
  }

  /* validate genesis (special case) */
  if (prev_block == nullptr) {
    return (
      block->index == 0 &&
      block->hash == block->Hash()
    );
  }

  /* ensure cronological indices */
  if (block->index != prev_block->index + 1) {
    return false;
  }

  /* the prev_hash must match the hash of the previous block */
  if (block->prev_hash != prev_block->hash) {
    return false;
  } 

  /* the block's hash must match itself */
  if (block->hash != block->Hash()) {
    return false;
  }

  return true;

}

bool Blockchain::IsValidChain() {
  return true;
}

void Blockchain::AcceptBlock(Block& block) {


}
