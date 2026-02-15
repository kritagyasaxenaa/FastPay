#ifndef AES_HELPER_H
#define AES_HELPER_H

#include <string>
#include <vector>

std::vector<unsigned char> aesEncrypt(const std::string& plaintext,
                                      const std::vector<unsigned char>& key,
                                      const std::vector<unsigned char>& iv);

std::vector<unsigned char> aesDecryptWithEmbeddedIV(const std::vector<unsigned char>& data,
                                                   const std::vector<unsigned char>& key);

std::vector<unsigned char> prepareAndEncrypt(int senderId,
                                            const std::vector<float>& ultrasoundData,
                                            const std::vector<unsigned char>& key32);

#endif
