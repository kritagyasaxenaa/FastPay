#include "DigitalSignature.h"
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/ec.h>
#include <openssl/bio.h>
#include <chrono>
#include <sstream>
#include <iomanip>

namespace DigitalSignature {

long long getTimestampNonce() {
    return std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

std::string getTimestampNonceString() {
    auto now = std::chrono::system_clock::now();
    auto t = std::chrono::system_clock::to_time_t(now);
    std::tm* tm = std::localtime(&t);
    if (!tm) return std::to_string(getTimestampNonce());
    std::ostringstream oss;
    oss << std::put_time(tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

void generateKeyPair(std::string& pubKeyPem, std::string& privKeyPem) {
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, nullptr);
    if (!ctx) return;

    EVP_PKEY* pkey = nullptr;
    if (EVP_PKEY_keygen_init(ctx) != 1) { EVP_PKEY_CTX_free(ctx); return; }
    if (EVP_PKEY_CTX_set_ec_paramgen_curve_nid(ctx, NID_X9_62_prime256v1) != 1) {
        EVP_PKEY_CTX_free(ctx); return;
    }
    if (EVP_PKEY_keygen(ctx, &pkey) != 1) { EVP_PKEY_CTX_free(ctx); return; }
    EVP_PKEY_CTX_free(ctx);

    BIO* bioPub = BIO_new(BIO_s_mem());
    BIO* bioPriv = BIO_new(BIO_s_mem());
    PEM_write_bio_PUBKEY(bioPub, pkey);
    PEM_write_bio_PrivateKey(bioPriv, pkey, nullptr, nullptr, 0, nullptr, nullptr);

    char* pubData = nullptr;
    char* privData = nullptr;
    long pubLen = BIO_get_mem_data(bioPub, &pubData);
    long privLen = BIO_get_mem_data(bioPriv, &privData);
    if (pubData && pubLen > 0) pubKeyPem.assign(pubData, static_cast<size_t>(pubLen));
    if (privData && privLen > 0) privKeyPem.assign(privData, static_cast<size_t>(privLen));

    BIO_free_all(bioPub);
    BIO_free_all(bioPriv);
    EVP_PKEY_free(pkey);
}

std::vector<unsigned char> signTransaction(const std::string& message, const std::string& privKeyPem) {
    BIO* bio = BIO_new_mem_buf(privKeyPem.data(), static_cast<int>(privKeyPem.size()));
    EVP_PKEY* pkey = PEM_read_bio_PrivateKey(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);
    if (!pkey) return {};

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) { EVP_PKEY_free(pkey); return {}; }

    std::vector<unsigned char> sig;
    size_t sigLen = 0;
    if (EVP_DigestSignInit(ctx, nullptr, EVP_sha256(), nullptr, pkey) != 1) goto cleanup;
    if (EVP_DigestSignUpdate(ctx, message.data(), message.size()) != 1) goto cleanup;
    if (EVP_DigestSignFinal(ctx, nullptr, &sigLen) != 1 || sigLen == 0) goto cleanup;
    sig.resize(sigLen);
    if (EVP_DigestSignFinal(ctx, sig.data(), &sigLen) != 1) { sig.clear(); goto cleanup; }
    sig.resize(sigLen);

cleanup:
    EVP_MD_CTX_free(ctx);
    EVP_PKEY_free(pkey);
    return sig;
}

bool verifySignature(const std::string& message,
                     const std::vector<unsigned char>& signature,
                     const std::string& pubKeyPem) {
    BIO* bio = BIO_new_mem_buf(pubKeyPem.data(), static_cast<int>(pubKeyPem.size()));
    EVP_PKEY* pkey = PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);
    if (!pkey) return false;

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) { EVP_PKEY_free(pkey); return false; }

    bool ok = (EVP_DigestVerifyInit(ctx, nullptr, EVP_sha256(), nullptr, pkey) == 1 &&
              EVP_DigestVerifyUpdate(ctx, message.data(), message.size()) == 1 &&
              EVP_DigestVerifyFinal(ctx, signature.data(), signature.size()) == 1);

    EVP_MD_CTX_free(ctx);
    EVP_PKEY_free(pkey);
    return ok;
}

} // namespace DigitalSignature
