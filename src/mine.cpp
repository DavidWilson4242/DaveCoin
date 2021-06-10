#include "mine.hpp"

bool Miner::IsMined(const Block& b) {
  
  if (b.bit_thresh > b.hash.size()) {
    return false;
  }

  for (int i = 0; i < b.bit_thresh; i++) {
    if (b.hash[i] != '0') {
      return false;
    }
  } 

  return true;

}

void Miner::MineBlock(Block& b) {
  
  while (true) {

    for (b.nonce = 0; b.nonce < UINT32_MAX; b.nonce++) {
      b.Hash();

      if (Miner::IsMined(b)) {
	break;
      }
    }  

    b.timestamp = std::time(0);
  }

}

void Miner::BroadcastMinedBlock(Block& b) {

  

}
