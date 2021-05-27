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
  

}

std::string Block::Serialize() {
  
  std::string

}

Blockchain::Blockchain() {
  
  /* initialize the genesis block.  this is always the same */
  genesis = std::shared_ptr<Block>(new Block);
  genesis->index = 0;
  genesis->timestamp = time(NULL);
  genesis->nonce = 0;
  genesis->bit_thresh = 3;

}

void Blockchain::AddBlock(Block& b) {


}


CryptoPP::SHA256& Block::Hash() {

  return hash;
  
}

bool Block::IsSolved() {
  return true;
}

void Block::Mine() {

}

bool Blockchain::IsValidBlock(Block *block, Block *prev_block) {
  return true;
}

bool Blockchain::IsValidChain() {
  return true;
}

void Blockchain::AcceptBlock(Block& block) {

}
