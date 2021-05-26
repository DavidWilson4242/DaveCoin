#include <sstream>
#include <ctime>
#include <string>
#include "tx.hpp"

std::string Tx_Input::Serialize() const {
  return "";
}

std::string Tx_Output::Serialize() const {
  return "";
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

/* verifies that the transaction was:
   1. signed by the holder of the private key associated with origin
   2. message is the exact same as the sender intended
*/
bool Tx::Verify() const {
  return Sig::ValidateSignature(Serialize() + hash, sig, origin); 
};

/* this function is used specifically to create a transaction that
   I, the DaveCoin holder, want to send */
Tx Tx::ConstructTransaction(std::vector<Tx_Input>& inputs,
		 	    std::vector<Tx_Output>& outputs,
			    const DSA::PublicKey& public_key,
			    const DSA::PrivateKey& private_key) {
  
  Tx tx; 
  
  tx.inputs = inputs;
  tx.outputs = outputs;
  tx.origin = public_key;
  tx.version = 0;
  tx.timestamp = std::time(0);
  
  /* serialize for hash-generation and signature generation */
  std::string message = tx.Serialize();
  SHA256 sha;

  sha.Update(reinterpret_cast<const byte *>(message.data()), message.size());
  tx.hash.resize(sha.DigestSize());
  sha.Final((byte *)&tx.hash[0]);

  /* verify that this transaction is properly signed */
  tx.sig = Sig::SignMessage(message+tx.hash, public_key, private_key);

  return tx;
  
}

/* decodes a serialized transaction into an object and performs verification */
Tx Tx::Decode(const std::string& serial) {

  Tx tx;
  size_t cr;
  uint32_t version, timestamp, keysize, input_size, output_size;
  const char *sdata = serial.c_str();
  
  version   = *reinterpret_cast<const uint32_t *>(&sdata[0]);
  timestamp = *reinterpret_cast<const uint32_t *>(&sdata[4]);
  keysize   = *reinterpret_cast<const uint32_t *>(&sdata[8]); 

  /* read public key */
  cr = 12;
  std::vector<byte> key_material(&sdata[cr], &sdata[cr] + keysize);
  ArraySource key_source(&key_material[0], key_material.size(), true);
  tx.origin.BERDecode(key_source);
  cr += keysize;

  /* read input size */
  input_size = *reinterpret_cast<const uint32_t *>(&sdata[cr]);
  cr += 4;
  
  std::cout << "INPUT SIZE " << input_size << std::endl;
  
  tx.version = version;
  tx.timestamp = timestamp;

  return tx;

}
