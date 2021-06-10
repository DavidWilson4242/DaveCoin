#include <sstream>
#include <ctime>
#include <string>
#include "tx.hpp"

std::string Tx_Input::Serialize() const {

  std::stringstream s;
  
  uint32_t blocksize = block_hash.size();
  uint32_t inputsize = input_hash.size();
  
  /* write block hash size, block hash */
  s.write(reinterpret_cast<const char *>(&blocksize), sizeof(uint32_t));
  s << block_hash;

  /* write tx hash size, tx hash */
  s.write(reinterpret_cast<const char *>(&inputsize), sizeof(uint32_t));
  s << input_hash;
  
  /* write index */
  s.write(reinterpret_cast<const char *>(&index), sizeof(uint32_t));

  return s.str();

}

Tx_Input Tx_Input::Decode(const std::string& serial, size_t *cr = nullptr) {
  
  Tx_Input txi;
  uint32_t blocksize, inputsize;
  const char *sdata = serial.c_str();
  size_t cr_dummy = 0;
  
  /* cursor into the string is optional.  use 0 if not passed */
  if (cr == nullptr) {
    cr = &cr_dummy;
  }
  
  /* read block hash size, block hash */
  blocksize = *reinterpret_cast<const uint32_t *>(&sdata[*cr]);
  *cr += 4;
  txi.block_hash = std::string(&sdata[*cr], blocksize);
  *cr += blocksize; 

  /* read tx hash size, tx hash */
  inputsize = *reinterpret_cast<const uint32_t *>(&sdata[*cr]);
  *cr += 4;
  txi.input_hash = std::string(&sdata[*cr], inputsize);
  *cr += inputsize;

  /* read index */
  txi.index = *reinterpret_cast<const uint32_t *>(&sdata[*cr]);
  *cr += 4;

  return txi;

}

std::string Tx_Output::Serialize() const {

  std::stringstream s;
  
  /* write target key size, target key */
  std::string target_string;
  uint32_t keysize;
  target.Save(StringSink(target_string).Ref());
  keysize = (uint32_t)target_string.size();
  s.write(reinterpret_cast<const char *>(&keysize), sizeof(uint32_t));
  s << target_string;

  /* write index */
  s.write(reinterpret_cast<const char *>(&index), sizeof(uint32_t));

  /* write coin amount */
  s.write(reinterpret_cast<const char *>(&coins), sizeof(uint64_t));

  return s.str();

}

Tx_Output Tx_Output::Decode(const std::string& serial, size_t *cr = nullptr) {

  Tx_Output txo;
  uint32_t keysize;
  const char *sdata = serial.c_str();
  size_t cr_dummy = 0;
  
  /* cursor into the string is optional.  use 0 if not passed */
  if (cr == nullptr) {
    cr = &cr_dummy;
  }

  /* read target size, target public key */
  keysize = *reinterpret_cast<const uint32_t *>(&sdata[*cr]);
  *cr += 4;
  std::vector<byte> key_material(&sdata[*cr], &sdata[*cr] + keysize);
  ArraySource key_source(&key_material[0], key_material.size(), true);
  txo.target.BERDecode(key_source);
  *cr += keysize;

  /* read index */
  txo.index = *reinterpret_cast<const uint32_t *>(&sdata[*cr]);
  *cr += 4;
  
  /* read coins */
  txo.coins = *reinterpret_cast<const uint32_t *>(&sdata[*cr]);
  *cr += 4;

  return txo;

}

std::string Tx::Serialize() const {
  
  std::stringstream s;
  const uint32_t input_size = (uint32_t)inputs.size();
  const uint32_t output_size = (uint32_t)outputs.size();
  
  /* write version, timestamp */
  s.write(reinterpret_cast<const char *>(&version), sizeof(uint32_t));
  s.write(reinterpret_cast<const char *>(&timestamp), sizeof(uint32_t));
  
  /* write public key size, key (who initiaited the transaction) */
  std::string origin_string;
  uint32_t keysize;
  origin.Save(StringSink(origin_string).Ref());
  keysize = (uint32_t)origin_string.size();
  s.write(reinterpret_cast<const char *>(&keysize), sizeof(uint32_t));
  s << origin_string;

  /* write number of inputs */
  s.write(reinterpret_cast<const char *>(&input_size), sizeof(uint32_t));
  
  /* write input data */ 
  for (auto& input: inputs) {
    s << input.Serialize();
  }
  
  /* write number of outputs */
  s.write(reinterpret_cast<const char *>(&output_size), sizeof(uint32_t));
  
  /* write output data */
  for (auto& output: outputs) {
    s << output.Serialize();
  }

  return s.str();

}

/* decodes a serialized transaction into an object and performs verification */
/* cr is an optional cursor into the serial string (i.e if we're serializing
 * a bigger string that contains a sub-transaction serial, we can use cr
 * to point to the index we care about, and the value *cr will be incremented
 * for use in the outer decode function */
Tx Tx::Decode(const std::string& serial, size_t *cr = nullptr) {

  Tx tx;
  uint32_t version, timestamp, keysize, input_size, output_size;
  const char *sdata = serial.c_str();
  size_t dummy_cr = 0;
  
  if (cr == nullptr) {
    cr = &dummy_cr;
  }
  
  version   = *reinterpret_cast<const uint32_t *>(&sdata[*cr]);
  timestamp = *reinterpret_cast<const uint32_t *>(&sdata[*cr + 4]);
  keysize   = *reinterpret_cast<const uint32_t *>(&sdata[*cr + 8]); 

  /* read public key */
  *cr += 12;
  std::vector<byte> key_material(&sdata[*cr], &sdata[*cr] + keysize);
  ArraySource key_source(&key_material[0], key_material.size(), true);
  tx.origin.BERDecode(key_source);
  *cr += keysize;

  /* read input size */
  input_size = *reinterpret_cast<const uint32_t *>(&sdata[*cr]);
  *cr += 4;

  /* read inputs */
  for (int i = 0; i < input_size; i++) {
    tx.inputs.push_back(Tx_Input::Decode(serial, cr));
  }

  /* read output size */
  output_size = *reinterpret_cast<const uint32_t *>(&sdata[*cr]);
  *cr += 4;

  /* read outputs */
  for (int i = 0; i < output_size; i++) {
    tx.outputs.push_back(Tx_Output::Decode(serial, cr));
  }
  
  tx.version = version;
  tx.timestamp = timestamp;

  /* generate transaction hash */
  tx.hash = Sig::CalculateSHA256(serial);
  
  return tx;

}

/* decodes a serialized transaction, requiring the public key and signature */
Tx Tx::DecodeAndVerify(const std::string& serial,
                       const std::string& signature,
		       const DSA::PublicKey& key) {
  
  Tx tx = Tx::Decode(serial); 

  if (!Sig::ValidateSignature(serial+tx.hash, signature, key)) {
    throw std::runtime_error("Failed to DecodeAndVerify Tx");
  }

  tx.sig = signature;
  return tx;

}

/* verifies that the transaction was:
   1. signed by the holder of the private key associated with origin
   2. message is the exact same as the sender intended
*/
bool Tx::Verify() const {
  return Sig::ValidateSignature(Serialize() + hash, sig, origin); 
};

/* this function is used specifically to create a transaction that
   I, the DaveCoin holder, want to send */
Tx Tx::ConstructAndSign(std::vector<Tx_Input>& inputs,
		 	std::vector<Tx_Output>& outputs,
			const DSA::PublicKey& public_key,
			const DSA::PrivateKey& private_key) {
  
  Tx tx; 
  CryptoPP::SHA256 sha;
  
  tx.inputs = inputs;
  tx.outputs = outputs;
  tx.origin = public_key;
  tx.version = 0;
  tx.timestamp = std::time(0);

  /* verify the transaction */
  if (inputs.size() == 0) {
    throw std::runtime_error("Transaction requires at least one input");
  }
  if (outputs.size() == 0) {
    throw std::runtime_error("Transaction requires at least one output");
  }
  for (const auto& output: tx.outputs) {
    if (output.target == public_key) {
      throw std::runtime_error("Transaction can't be to myself");
    } 
  }

  /* serialize for hash-generation and signature generation */
  std::string message = tx.Serialize();
  sha.Update(reinterpret_cast<const byte *>(message.data()), message.size());
  tx.hash.resize(sha.DigestSize());
  sha.Final((byte *)&tx.hash[0]);

  /* verify that this transaction is properly signed */
  tx.sig = Sig::SignMessage(message+tx.hash, public_key, private_key);

  return tx;
  
}
