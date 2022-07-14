#include "../aes256/include/aes256.hpp"

#pragma comment (lib, "../x64/Release/aes256.lib")

void aesEncryption(const unsigned char* plainData, int plainDataLength, unsigned char* encryptionData, int& encryptionDataLength);
void aesDecryption(const unsigned char* encryptedData, int encryptedDataLength, unsigned char* decryptionData, int& decryptionDataLength);