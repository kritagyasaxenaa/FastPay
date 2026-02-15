#include "CryptoHandler.h"
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <cstdlib>

void CryptoHandler::generateKeyPair(std::string& pubKey, std::string& privKey) {
    const int bits = 2048;
        BIGNUM* bn = BN_new();
    BN_set_word(bn, RSA_F4);
    RSA* rsa = RSA_new();
    if (!RSA_generate_key_ex(rsa, bits, bn, nullptr)) {
        RSA_free(rsa);
        BN_free(bn);
        return;
    }
    BIO* pub = BIO_new(BIO_s_mem());
    PEM_write_bio_RSA_PUBKEY(pub, rsa);
    char* pub_data = nullptr;
    long pub_len = BIO_get_mem_data(pub, &pub_data);
    if (pub_data && pub_len > 0) pubKey.assign(pub_data, static_cast<size_t>(pub_len));
    BIO_free_all(pub);

    BIO* priv = BIO_new(BIO_s_mem());
    PEM_write_bio_RSAPrivateKey(priv, rsa, nullptr, nullptr, 0, nullptr, nullptr);
    char* priv_data = nullptr;
    long priv_len = BIO_get_mem_data(priv, &priv_data);
    if (priv_data && priv_len > 0) privKey.assign(priv_data, static_cast<size_t>(priv_len));
    BIO_free_all(priv);

    RSA_free(rsa);
    BN_free(bn);
}

std::vector<unsigned char> CryptoHandler::encrypt(const std::string& plainText, const std::string& pubKey) {
    BIO* bio = BIO_new_mem_buf(pubKey.data(), static_cast<int>(pubKey.size()));
    RSA* rsa = PEM_read_bio_RSA_PUBKEY(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);
    if (!rsa) return {};

    std::vector<unsigned char> encrypted(static_cast<size_t>(RSA_size(rsa)));
    int result = RSA_public_encrypt(static_cast<int>(plainText.size()),
                                    reinterpret_cast<const unsigned char*>(plainText.c_str()),
                                    encrypted.data(), rsa, RSA_PKCS1_OAEP_PADDING);
    RSA_free(rsa);
    if (result <= 0) return {};
    encrypted.resize(static_cast<size_t>(result));
    return encrypted;
}

std::string CryptoHandler::decrypt(const std::vector<unsigned char>& cipherText, const std::string& privKey) {
    BIO* bio = BIO_new_mem_buf(privKey.data(), static_cast<int>(privKey.size()));
    RSA* rsa = PEM_read_bio_RSAPrivateKey(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);
    if (!rsa) return "DECRYPTION_FAILED";

    std::vector<unsigned char> decrypted(static_cast<size_t>(RSA_size(rsa)));
    int result = RSA_private_decrypt(static_cast<int>(cipherText.size()), cipherText.data(),
                                     decrypted.data(), rsa, RSA_PKCS1_OAEP_PADDING);
    RSA_free(rsa);
    if (result <= 0) return "DECRYPTION_FAILED";
    return std::string(reinterpret_cast<char*>(decrypted.data()), static_cast<size_t>(result));
}