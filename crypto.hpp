#ifndef CRYPTO_HPP
#define CRYPTO_HPP

#include <vector>
#include <string>
#include <sstream>
#include <inttypes.h>
#include "sha256.h"

namespace Crypto {

  const int SHA256_BUFFER_SIZE = SHA256_BLOCK_SIZE;

  std::vector<uint8_t> SHA256(uint8_t *data, size_t length);
  std::string          SHA256_tostring(const std::vector<uint8_t>&);

}

#endif
