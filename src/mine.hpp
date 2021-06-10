#ifndef MINE_HPP
#define MINE_HPP

#include "blockchain.hpp"

/*
 *
 * This module is in charge of mining a block for a hash with
 * a certain number of leading 0s.  When found, we will tell
 * the server (server.cpp) to broadcast that block to clients.
 *
 * This module is also in charge of verifying a mined block
 * (i.e it was not tampered with, follows all rules, etc.)
 *
 * In the future, a more efficent mine module should be produced
 * that runs on the GPU.  This runs on the CPU, and is 
 * horrifically slow.  But it'll work for development.
 *
 *
 */

namespace Miner {
  
  bool IsMined(const Block& b); 
  void MineBlock(Block& b);
  void BroadcastMinedBlock(Block& b);

};

#endif
