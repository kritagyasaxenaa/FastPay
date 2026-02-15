#include "AES.h"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/aes.h>
#include <cstring>
#include <stdexcept>

static constexpr size_t AES_BLOCK_LEN = 16;
static constexpr size_t IV_SIZE = 16;
static constexpr size_t KEY_SIZE_BYTES = 32;

std::vector<unsigned char> aesEncrypt(const std::string& plaintext,
                                      const std::vector<unsigned char>& key,
                                      const std::vector<unsigned char>& iv) {
    if (key.size() != KEY_SIZE_BYTES || iv.size() != IV_SIZE)
        return {};

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return {};

    std::vector<unsigned char> ciphertext(plaintext.size() + AES_BLOCK_LEN);
    int len, ciphertext_len;

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key.data(), iv.data()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }
    if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len,
                          reinterpret_cast<const unsigned char*>(plaintext.c_str()),
                          static_cast<int>(plaintext.size())) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }
    ciphertext_len = len;
    if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }
    ciphertext_len += len;
    ciphertext.resize(ciphertext_len);
    EVP_CIPHER_CTX_free(ctx);
    return ciphertext;
}

std::vector<unsigned char> aesDecryptWithEmbeddedIV(const std::vector<unsigned char>& data,
                                                     const std::vector<unsigned char>& key) {
    if (data.size() < IV_SIZE || key.size() != KEY_SIZE_BYTES)
        return {};

    std::vector<unsigned char> iv(data.begin(), data.begin() + IV_SIZE);
    std::vector<unsigned char> ciphertext(data.begin() + IV_SIZE, data.end());

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return {};

    std::vector<unsigned char> plaintext(ciphertext.size());
    int len, plaintext_len;

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key.data(), iv.data()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }
    if (EVP_DecryptUpdate(ctx, plaintext.data(), &len, ciphertext.data(),
                          static_cast<int>(ciphertext.size())) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }
    plaintext_len = len;
    if (EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }
    plaintext_len += len;
    plaintext.resize(plaintext_len);
    EVP_CIPHER_CTX_free(ctx);
    return plaintext;
}

std::vector<unsigned char> prepareAndEncrypt(int senderId,
                                             const std::vector<float>& ultrasoundData,
                                             const std::vector<unsigned char>& key32) {
    if (key32.size() != KEY_SIZE_BYTES) return {};

    size_t payloadSize = sizeof(int) + ultrasoundData.size() * sizeof(float);
    std::vector<unsigned char> plaintext(payloadSize);
    std::memcpy(plaintext.data(), &senderId, sizeof(int));
    std::memcpy(plaintext.data() + sizeof(int), ultrasoundData.data(),
                ultrasoundData.size() * sizeof(float));

    unsigned char iv[IV_SIZE];
    if (RAND_bytes(iv, IV_SIZE) != 1) return {};

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return {};

    std::vector<unsigned char> ciphertext(plaintext.size() + AES_BLOCK_LEN);
    int len, ciphertext_len;

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key32.data(), iv) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }
    EVP_EncryptUpdate(ctx, ciphertext.data(), &len, plaintext.data(), static_cast<int>(plaintext.size()));
    ciphertext_len = len;
    EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len);
    ciphertext_len += len;
    ciphertext.resize(ciphertext_len);
    EVP_CIPHER_CTX_free(ctx);

    std::vector<unsigned char> out;
    out.insert(out.end(), iv, iv + IV_SIZE);
    out.insert(out.end(), ciphertext.begin(), ciphertext.end());
    return out;
}
