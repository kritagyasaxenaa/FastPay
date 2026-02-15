#include "CryptoHandler.h"
#include "Ultrasound.h"
#include "DigitalSignature.h"
#include <iostream>
#include <string>
#include <vector>
#include <cstring>

// --- Online flow (receiver emits ultrasound public key; sender captures 2x, extracts key, initiates with PIN) ---
int runOnlineFlow() {
    std::string receiverPubKey, receiverPrivKey;
    CryptoHandler::generateKeyPair(receiverPubKey, receiverPrivKey);

    // Receiver: build ultrasound payload [header (n)][public key (x-n)] = x
    std::vector<unsigned char> header(Ultrasound::HEADER_SIZE, 0);
    const char* ident = "FASTPAY_ONLINE_V1";
    size_t idLen = std::strlen(ident);
    if (idLen > header.size()) idLen = header.size();
    std::memcpy(header.data(), ident, idLen);

    std::vector<unsigned char> pubKeyBytes(receiverPubKey.begin(), receiverPubKey.end());
    if (pubKeyBytes.size() > Ultrasound::KEY_SIZE) pubKeyBytes.resize(Ultrasound::KEY_SIZE);
    std::vector<unsigned char> emitPayload = Ultrasound::buildEmitPayload(header, pubKeyBytes);
    if (emitPayload.empty()) {
        std::cerr << "Failed to build emit payload\n";
        return -1;
    }
    std::cout << "Receiver: emitting " << emitPayload.size() << " bytes (header + pubkey)\n";

    // Sender: simulate mic buffer 2x (in real app, fill from mic)
    std::vector<unsigned char> micBuffer(2 * emitPayload.size(), 0);
    size_t offset = 0;
    if (micBuffer.size() >= emitPayload.size())
        std::memcpy(micBuffer.data() + offset, emitPayload.data(), emitPayload.size());

    std::vector<unsigned char> extractedKey = Ultrasound::extractKeyFromMic(micBuffer, header);
    if (extractedKey.empty()) {
        std::cerr << "Sender: could not extract key from mic buffer\n";
        return -1;
    }
    // Trim to valid PEM (in case of padding zeros)
    std::string keyStr(extractedKey.begin(), extractedKey.end());
    size_t endMark = keyStr.find("-----END PUBLIC KEY-----");
    if (endMark != std::string::npos)
        keyStr = keyStr.substr(0, endMark + 24);
    std::string senderExtractedPubKey = keyStr;

    // Sender: build transaction payload (e.g. UPI ID + amount + nonce) and encrypt with receiver's public key
    std::string nonce = DigitalSignature::getTimestampNonceString();
    std::string upiId = "sender@fastpay";
    std::string amount = "100.00";
    std::string payload = upiId + "|" + amount + "|" + nonce;

    std::vector<unsigned char> encrypted = CryptoHandler::encrypt(payload, senderExtractedPubKey);
    if (encrypted.empty()) {
        std::cerr << "Sender: encryption failed (key may be truncated)\n";
        return -1;
    }
    std::cout << "Sender: encrypted payload size " << encrypted.size() << " bytes. Initiate online with PIN.\n";

    // Server/receiver: decrypt (simulated here)
    std::string decrypted = CryptoHandler::decrypt(encrypted, receiverPrivKey);
    std::cout << "Receiver/Server: transaction received: " << decrypted << std::endl;
    return 0;
}

// --- Offline flow (cold wallet to cold wallet: sender signs, receiver verifies and sends receipt; sync when online) ---
int runOfflineFlow() {
    std::string senderPubKey, senderPrivKey;
    std::string receiverPubKey, receiverPrivKey;
    DigitalSignature::generateKeyPair(senderPubKey, senderPrivKey);
    DigitalSignature::generateKeyPair(receiverPubKey, receiverPrivKey);

    std::string nonce = DigitalSignature::getTimestampNonceString();
    std::string senderId = "cold_sender@fastpay";
    std::string receiverId = "cold_receiver@fastpay";
    std::string amount = "50.00";
    std::string message = senderId + "|" + receiverId + "|" + amount + "|" + nonce;

    std::vector<unsigned char> signature = DigitalSignature::signTransaction(message, senderPrivKey);
    if (signature.empty()) {
        std::cerr << "Offline: sender sign failed\n";
        return -1;
    }
    std::cout << "Offline: sender signed transaction (nonce=" << nonce << ")\n";

    bool verified = DigitalSignature::verifySignature(message, signature, senderPubKey);
    if (!verified) {
        std::cerr << "Offline: verification failed – transaction invalid, account frozen\n";
        return -1;
    }
    std::cout << "Offline: receiver verified sender signature\n";

    std::string receiptMessage = message + "|RECEIPT";
    std::vector<unsigned char> receiptSig = DigitalSignature::signTransaction(receiptMessage, receiverPrivKey);
    if (receiptSig.empty()) {
        std::cerr << "Offline: receiver receipt sign failed\n";
        return -1;
    }
    std::cout << "Offline: receiver emitted receipt signature. Sync to server when online.\n";
    return 0;
}

int main() {
    std::cout << "=== Online (receiver emits ultrasound key; sender pays with PIN) ===\n";
    runOnlineFlow();
    std::cout << "\n=== Offline (cold wallet: sign -> verify -> receipt) ===\n";
    runOfflineFlow();
    return 0;
}
