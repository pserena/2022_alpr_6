#include "AesManager.h"

//------------------------------------------------------------------------------
// AES256 encryption function                      
//                                                 
// * parameter                                       
//  1) plainData : data to be encrypted             
//  2) plainDataLength : length of data to be encrypted 
//  3) encryptionData : buffer to store encrypted data                   
//   -> It should have enough space.
//   -> Encrypted data is in units of 16 bytes. 1 byte is added to the data.
//   -> For example, if plainData is 8 bytes, encryptionData will be 17 bytes. 
//   -> For example, If plainData is 32 bytes, encryptionData will be 33 bytes.                                    
//  4) encryptionDataLength : length of encrypted data
//
// * return : void
//------------------------------------------------------------------------------
void aesEncryption(const unsigned char* plainData, int plainDataLength, unsigned char* encryptionData, int& encryptionDataLength)
{
    ByteArray aesKey(AES_KEY_SIZE);
    ByteArray encryptedData;
    // copy AES256 private key
    for (int i = 0; i < AES_KEY_SIZE; i++)
    {
        aesKey[i] = aesPrivateKeyOnlyForServer[i];
    }
    // AES256 encryption
    Aes256::encrypt(aesKey, plainData, plainDataLength, encryptedData);

    encryptionDataLength = encryptedData.size();
    // store encrypted data to buffer
    for (int i = 0; i < encryptionDataLength; i++)
    {
        encryptionData[i] = encryptedData.at(i);
    }

    return;
}

//------------------------------------------------------------------------------
// AES256 decryption function                      
//                                                 
// * parameter                                       
//  1) encryptedData : encryption data to be decrypted             
//  2) encryptedDataLength : length of encryption data 
//  3) decryptionData : buffer to store decrypted data                   
//   -> Maximum length of decryptionData is (encryptedDataLength - 1) bytes.               
//  4) decryptionDataLength : length of decrypted data
//
// * return : void
//------------------------------------------------------------------------------
void aesDecryption(const unsigned char* encryptedData, int encryptedDataLength, unsigned char* decryptionData, int& decryptionDataLength)
{
    ByteArray aesKey(AES_KEY_SIZE);
    ByteArray decryptedData;
    // copy AES256 private key
    for (int i = 0; i < AES_KEY_SIZE; i++)
    {
        aesKey[i] = aesPrivateKeyOnlyForServer[i];
    }
    // AES256 decryption
    Aes256::decrypt(aesKey, encryptedData, encryptedDataLength, decryptedData);

    decryptionDataLength = decryptedData.size();
    // store decrypted data to buffer
    for (int i = 0; i < decryptionDataLength; i++)
    {
        decryptionData[i] = decryptedData.at(i);
    }
    decryptionData[decryptedData.size()] = 0;

    return;
}
