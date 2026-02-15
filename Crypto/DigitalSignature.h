#ifndef DIGITAL_SIGNATURE_H
#define DIGITAL_SIGNATURE_H

#include <string>
#include <vector>

namespace DigitalSignature {

// Nonce = timestamp (date + time) for all transactions
long long getTimestampNonce();
std::string getTimestampNonceString();  // "YYYY-MM-DD HH:MM:SS" form

// ECDSA (or RSA) sign: message = e.g. "senderId|receiverId|amount|nonce"
std::vector<unsigned char> signTransaction(const std::string& message, const std::string& privKeyPem);

bool verifySignature(const std::string& message,
                     const std::vector<unsigned char>& signature,
                     const std::string& pubKeyPem);

// Generate ECDSA key pair (PEM strings)
void generateKeyPair(std::string& pubKeyPem, std::string& privKeyPem);

} // namespace DigitalSignature

#endif
