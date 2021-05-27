#include <cryptopp/filters.h>
#include "sig.hpp"

using namespace CryptoPP;

/* generates a DSA pair of public & private keys */
std::tuple<DSA::PublicKey, DSA::PrivateKey>
Sig::GenerateKeys() {
  
  AutoSeededRandomPool rng;

  /* make private key */
  DSA::PrivateKey private_key;
  private_key.GenerateRandomWithKeySize(rng, 1024);

  /* make public key */
  DSA::PublicKey public_key;
  public_key.AssignFrom(private_key);
  if (!private_key.Validate(rng, 3) || !public_key.Validate(rng, 3)) {
    throw std::runtime_error("DSA generation failed");
  }

  return {public_key, private_key};

}

/* cryptographically signs a message given public & private keypair */
std::string Sig::SignMessage(const std::string &message,
			     const DSA::PublicKey &public_key,
			     const DSA::PrivateKey &private_key) {
  
  std::string signature;
  DSA::Signer signer(private_key);

  AutoSeededRandomPool rng;

  StringSource ss1(message, true,
    new SignerFilter(rng, signer,
      new StringSink(signature)
    )
  );

  return signature;

} 

/* performs signature validation: was this message properly signed
   by the owner of the private_key associated with public_key? */
bool Sig::ValidateSignature(const std::string& message,
			    const std::string& signature,
			    const DSA::PublicKey& public_key) {
  DSA::Verifier verifier(public_key);

  bool result = false;
  StringSource ss(message + signature, true,
    new SignatureVerificationFilter(
      verifier,
      new ArraySink(reinterpret_cast<uint8_t *>(&result), sizeof(result)),
      SignatureVerificationFilter::PUT_RESULT | 
      SignatureVerificationFilter::SIGNATURE_AT_END
    )
  );

  return result;

}
