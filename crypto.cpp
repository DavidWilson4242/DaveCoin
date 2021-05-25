#include <iomanip>
#include <iostream>
#include "crypto.hpp"

std::vector<uint8_t> Crypto::SHA256(uint8_t *data, size_t length) {
  
  uint8_t buffer[SHA256_BLOCK_SIZE];

  SHA256_CTX ctx;
  sha256_init(&ctx);
  sha256_update(&ctx, data, length);
  sha256_final(&ctx, buffer);

  std::vector<uint8_t> hashed;
  hashed.reserve(SHA256_BLOCK_SIZE);

  for (int i = 0; i < SHA256_BLOCK_SIZE; i++) {
    hashed.push_back(buffer[i]);
  }
  
  return hashed;

}

std::string Crypto::SHA256_tostring(const std::vector<uint8_t> &hash) {
  std::stringstream stream;

  for (uint8_t byte: hash) {
    stream << std::hex << std::setw(2) << std::setfill('0') << (int)byte;
  }

  return stream.str();
}
