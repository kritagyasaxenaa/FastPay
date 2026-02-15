#ifndef ULTRASOUND_H
#define ULTRASOUND_H

#include <vector>

namespace Ultrasound {

// Total emitted size = HEADER_SIZE + KEY_SIZE (x = n + (x-n))
inline constexpr size_t HEADER_SIZE = 32;   // n: identifier/padding
inline constexpr size_t KEY_SIZE = 2208;   // x - n: public key payload (e.g. 2048-bit RSA PEM ~2240, adjust as needed)
inline constexpr size_t TOTAL_EMIT_SIZE = HEADER_SIZE + KEY_SIZE;  // x
inline constexpr size_t MIC_BUFFER_SIZE = 2 * TOTAL_EMIT_SIZE;     // 2x

// Extract receiver's public key from mic buffer (size 2x). Searches for padding (header) and returns key bytes after it.
std::vector<unsigned char> extractKeyFromMic(const std::vector<unsigned char>& micBuffer,
                                              const std::vector<unsigned char>& headerPadding);

// Build payload to emit: [header (n bytes)][publicKey (x-n bytes)] = total x bytes
std::vector<unsigned char> buildEmitPayload(const std::vector<unsigned char>& headerPadding,
                                            const std::vector<unsigned char>& publicKeyBytes);

} // namespace Ultrasound

#endif
