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
  bit_thresh = 0;
  

}

std::string Block::Serialize() {
  
  std::stringstream s;

  /* write block index */
  s.write(reinterpret_cast<const char *>(&index), sizeof(uint64_t));
  
  /* write previous hash size & prev_hash */
  uint32_t hash_size = prev_hash.size();
  s.write(reinterpret_cast<const char *>(&hash_size), sizeof(uint32_t));
  s << prev_hash;

  /* write nonce */
  s.write(reinterpret_cast<const char *>(&nonce), sizeof(uint32_t));

  /* write timestamp */
  s.write(reinterpret_cast<const char *>(&timestamp), sizeof(uint64_t));

  /* write coinbase key size, coinbase key */
  std::string coinbase_string;
  uint32_t keysize;
  coinbase.target.Save(StringSink(coinbase_string).Ref());
  keysize = (uint32_t)coinbase_string.size();
  s.write(reinterpret_cast<const char *>(&keysize), sizeof(uint32_t));
  s << coinbase_string;

  /* write coinbase amount */
  s.write(reinterpret_cast<const char *>(&coinbase.coins), sizeof(uint64_t));

  /* write number of transactions */
  uint32_t tx_size = tx.size();
  s.write(reinterpret_cast<const char *>(&tx_size), sizeof(uint32_t));

  /* write tx */
  for (const auto& t: tx) {
    s << t.Serialize();
  }

  return s.str();

}

Block Block::DecodeBlock(const std::string& serial) {
  
  Block b;
  const char *data = serial.c_str();
  size_t cr = 0;

  uint64_t blockindex;
  uint32_t hashlen;
  std::string prev_hash;
  uint32_t nonce;
  uint64_t timestamp;
  uint32_t coinbase_keysize;
  std::string coinbase_key;
  uint64_t coinbase_coins;
  uint32_t nTx;
  std::vector<Tx> transactions;

  /* read block index */
  blockindex = *reinterpret_cast<const uint64_t *>(&data[cr]);
  cr += sizeof(uint64_t);

  /* read prev_hash */
  hashlen = *reinterpret_cast<const uint32_t *>(&data[cr]);
  prev_hash = std::string(&data[cr + 4], hashlen);
  cr += hashlen + sizeof(uint32_t);
  
  /* read nonce */
  nonce = *reinterpret_cast<const uint32_t *>(&data[cr]);
  cr += sizeof(uint32_t);

  /* read timestamp */
  timestamp = *reinterpret_cast<const uint64_t *>(&data[cr]);
  cr += sizeof(uint64_t);

  /* read coinbase key */
  coinbase_keysize = *reinterpret_cast<const uint32_t *>(&data[cr]);
  coinbase_key = std::string(&data[cr + 4], coinbase_keysize);
  cr += coinbase_keysize + sizeof(uint32_t);

  /* read coinbase amount */
  coinbase_coins = *reinterpret_cast<const uint64_t *>(&data[cr]);
  cr += sizeof(uint64_t);

  /* read num.txactions */
  nTx = *reinterpret_cast<const uint32_t *>(&data[cr]);
  cr += sizeof(uint32_t);

  /* read.txactions */
  for (int i = 0; i < nTx; i++) {
    transactions.push_back(Tx::Decode(serial, &cr)); 
  }

  /* build block */
  b.index = blockindex;
  b.prev_hash = prev_hash;
  b.nonce = nonce;
  b.timestamp = timestamp;
  b.tx = std::move(transactions);

  /* a bit more complex to build a public key from bytes... */
  StringSource source(coinbase_key, true);
  b.coinbase.target.BERDecode(source);
  b.coinbase.coins = coinbase_coins; 

  b.hash = b.Hash();

  return std::move(b);

}

Blockchain::Blockchain() {
  
  /* initialize the genesis block.  this is always the same */
  genesis = std::shared_ptr<Block>(new Block);
  genesis->index = 0;
  genesis->timestamp = time(NULL);
  genesis->nonce = 0;
  genesis->bit_thresh = 3;
  genesis->solved = true;

}

void Blockchain::AddBlock(Block& b) {


}


std::string Block::Hash() {

  hash = "0";
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
