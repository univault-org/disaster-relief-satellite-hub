#include "devices/local_communication.h"
#include <algorithm>
#include <random>
#include <stdexcept>

LocalCommunication::LocalCommunication() {}

LocalCommunication::~LocalCommunication() {}

bool LocalCommunication::initializeUltrasonic() {
    // Initialize the RiifUltrasonic instance with default parameters
    RiifUltrasonic::Parameters params;
    m_ultrasonic.setParameters(params);
    return true;
}

bool LocalCommunication::performKeyExchange() {
    // Generate a random key
    m_sharedKey.resize(32);
    std::random_device rd;
    std::generate(m_sharedKey.begin(), m_sharedKey.end(), std::ref(rd));
    
    // Encode and send the key
    std::string keyStr(m_sharedKey.begin(), m_sharedKey.end());
    auto encodedKey = m_ultrasonic.encode(keyStr);
    
    // Simulate receiving the encoded key
    auto receivedKeyVec = m_ultrasonic.decode(encodedKey);
    std::string receivedKeyStr(receivedKeyVec.begin(), receivedKeyVec.end());
    
    // In a real implementation, we would validate the received key
    // and perform additional steps for secure key exchange
    
    return receivedKeyStr == keyStr;
}

bool LocalCommunication::sendSecureData(const std::string& data) {
    if (m_sharedKey.empty()) {
        throw std::runtime_error("Shared key not set. Perform key exchange first.");
    }
    
    // Simple XOR encryption (for demonstration purposes only)
    std::vector<uint8_t> encryptedData(data.begin(), data.end());
    for (size_t i = 0; i < encryptedData.size(); ++i) {
        encryptedData[i] ^= m_sharedKey[i % m_sharedKey.size()];
    }
    
    m_lastSentData = m_ultrasonic.encode(std::string(encryptedData.begin(), encryptedData.end()));
    return true;
}

bool LocalCommunication::receiveSecureData(std::string& data) {
    if (m_sharedKey.empty()) {
        throw std::runtime_error("Shared key not set. Perform key exchange first.");
    }
    
    // In a real implementation, we would receive actual encoded data
    // For simulation, we'll use the last sent data
    auto decodedDataVec = m_ultrasonic.decode(m_lastSentData);
    std::string decodedData(decodedDataVec.begin(), decodedDataVec.end());
    
    // Simple XOR decryption (for demonstration purposes only)
    std::vector<uint8_t> decryptedData(decodedData.begin(), decodedData.end());
    for (size_t i = 0; i < decryptedData.size(); ++i) {
        decryptedData[i] ^= m_sharedKey[i % m_sharedKey.size()];
    }
    
    data = std::string(decryptedData.begin(), decryptedData.end());
    return true;
}
