#include "Ultrasound.h"
#include <algorithm>
#include <cstddef>

namespace Ultrasound {

std::vector<unsigned char> extractKeyFromMic(const std::vector<unsigned char>& micBuffer,
                                             const std::vector<unsigned char>& headerPadding) {
    if (headerPadding.empty() || micBuffer.size() < HEADER_SIZE + KEY_SIZE)
        return {};

    auto it = std::search(micBuffer.begin(), micBuffer.end(),
                          headerPadding.begin(), headerPadding.end());
    if (it == micBuffer.end())
        return {};

    size_t startIdx = std::distance(micBuffer.begin(), it) + headerPadding.size();
    if (startIdx + KEY_SIZE > micBuffer.size())
        return {};

    return std::vector<unsigned char>(micBuffer.begin() + startIdx,
                                     micBuffer.begin() + startIdx + KEY_SIZE);
}

std::vector<unsigned char> buildEmitPayload(const std::vector<unsigned char>& headerPadding,
                                            const std::vector<unsigned char>& publicKeyBytes) {
    if (headerPadding.size() != HEADER_SIZE || publicKeyBytes.size() > KEY_SIZE)
        return {};

    std::vector<unsigned char> out;
    out.reserve(TOTAL_EMIT_SIZE);
    out.insert(out.end(), headerPadding.begin(), headerPadding.end());
    out.insert(out.end(), publicKeyBytes.begin(), publicKeyBytes.end());
    if (out.size() < TOTAL_EMIT_SIZE)
        out.resize(TOTAL_EMIT_SIZE, 0);  // pad to x bytes
    return out;
}

} // namespace Ultrasound
