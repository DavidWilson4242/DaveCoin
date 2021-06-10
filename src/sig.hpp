#ifndef SIG_HPP
#define SIG_HPP

#include <utility>
#include <cryptopp/dsa.h>
#include <cryptopp/osrng.h>
#include <cryptopp/files.h>

using namespace CryptoPP;

namespace Sig {
  
  std::string CalculateSHA256(const std::string& data);

  std::tuple<DSA::PublicKey, DSA::PrivateKey> GenerateKeys();

  std::string SignMessage(const std::string& message, 
			  const DSA::PublicKey& public_key, 
			  const DSA::PrivateKey& private_key);

  bool ValidateSignature(const std::string& message,
			 const std::string& signature,
			 const DSA::PublicKey& public_key);

  template <typename T>
  void LoadKey(const std::string& filename, T& key);

  template <typename T>
  void SaveKey(const std::string& filename, T& key);

};

/* saves a DSA key to file */
template<typename T>
void Sig::SaveKey(const std::string& filename, T& key) {

  static_assert(   std::is_same<DSA::PublicKey, T>::value 
                || std::is_same<DSA::PrivateKey, T>::value,
		"LoadKey only accepts DSA::PublicKey or DSA::PrivateKey");

  ByteQueue queue;
  key.Save(queue);
  FileSink file(filename.c_str());
  queue.CopyTo(file);
  file.MessageEnd();

}

/* loads a DSA key from file */
template<typename T>
void Sig::LoadKey(const std::string& filename, T& key) {

  static_assert(   std::is_same<DSA::PublicKey, T>::value 
                || std::is_same<DSA::PrivateKey, T>::value,
		"LoadKey only accepts DSA::PublicKey or DSA::PrivateKey");

  ByteQueue queue;
  AutoSeededRandomPool rnd;
  FileSource file(filename.c_str(), true);
  file.TransferTo(queue);
  queue.MessageEnd();
  key.Load(queue);

  if (!key.Validate(rnd, 3)) {
    throw std::runtime_error("DSA key validation failed.\n");
  }

}

#endif
