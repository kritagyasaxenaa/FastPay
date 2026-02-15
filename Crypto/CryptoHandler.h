#ifndef CRYPTO_HANDLER_H
#define CRYPTO_HANDLER_H

#include <string>
#include <vector>

class CryptoHandler {
public:
    static void generateKeyPair(std::string& pubKey, std::string& privKey);
    static std::vector<unsigned char> encrypt(const std::string& plainText, const std::string& pubKey);
    static std::string decrypt(const std::vector<unsigned char>& cipherText, const std::string& privKey);
};

#endif
