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
  
  uint64_t attempts = 0;

  while (true) {

    for (b.nonce = 0; b.nonce < UINT32_MAX; b.nonce++) {
      b.Hash();

      if (Miner::IsMined(b)) {
	break;
      }
      
      /* every so often, check if we received and validated a new block
       * from another node.  if we did, then we should move onto the 
       * next block */
      attempts++;
      if (attempts % 10000 == 0) {
	break;
      }
    }  

    b.timestamp = std::time(0);
  }

}

void Miner::BroadcastMinedBlock(Block& b) {

  

}
