// #include "network/satellite_hub.h"
// #include "srpt_satellite.h"
// #include <iostream>

// SatelliteHub::SatelliteHub() : m_session(nullptr) {
//     std::cout << "SatelliteHub constructor called" << std::endl;
// }

// SatelliteHub::~SatelliteHub() {
//     std::cout << "SatelliteHub destructor called" << std::endl;
//     if (m_session) {
//         m_session->Disconnect();
//     }
// }

// bool SatelliteHub::initializeSRPT() {
//     std::cout << "Initializing SRPT protocol..." << std::endl;
//     try {
//         SRPT::Satellite::SatelliteConfig config;
//         config.setProvider(SRPT::Satellite::Provider::STARLINK);
//         m_session = SRPT::Satellite::CreateSatelliteSession(config);
//         if (m_session) {
//             std::cout << "SRPT initialized successfully" << std::endl;
//             return true;
//         } else {
//             std::cerr << "Failed to create SatelliteSession" << std::endl;
//             return false;
//         }
//     } catch (const std::exception& e) {
//         std::cerr << "Exception during SRPT initialization: " << e.what() << std::endl;
//         return false;
//     }
// }

// bool SatelliteHub::connectToSatellite() {
//     std::cout << "Connecting to satellite..." << std::endl;
//     if (!m_session) {
//         std::cerr << "Cannot connect: SatelliteSession is null" << std::endl;
//         return false;
//     }
//     try {
//         bool connected = m_session->Connect("starlink-1");
//         if (connected) {
//             std::cout << "Connected to satellite successfully" << std::endl;
//         } else {
//             std::cerr << "Failed to connect to satellite" << std::endl;
//         }
//         return connected;
//     } catch (const std::exception& e) {
//         std::cerr << "Exception during satellite connection: " << e.what() << std::endl;
//         return false;
//     }
// }

// bool SatelliteHub::sendData(const std::string& data) {
//     if (!m_session) {
//         std::cerr << "Cannot send data: Not connected to satellite" << std::endl;
//         return false;
//     }
//     std::cout << "Sending data: " << data << std::endl;
//     try {
//         auto stream = m_session->CreateSatelliteStream();
//         if (!stream) {
//             std::cerr << "Failed to create satellite stream" << std::endl;
//             return false;
//         }
//         SRPT::ByteVector byteData(data.begin(), data.end());
//         bool sent = stream->Write(byteData);
//         if (sent) {
//             std::cout << "Data sent successfully" << std::endl;
//         } else {
//             std::cerr << "Failed to send data" << std::endl;
//         }
//         return sent;
//     } catch (const std::exception& e) {
//         std::cerr << "Exception during data sending: " << e.what() << std::endl;
//         return false;
//     }
// }

// bool SatelliteHub::receiveData(std::string& data) {
//     if (!m_session) {
//         std::cerr << "Cannot receive data: Not connected to satellite" << std::endl;
//         return false;
//     }
//     try {
//         auto stream = m_session->CreateSatelliteStream();
//         if (!stream) {
//             std::cerr << "Failed to create satellite stream" << std::endl;
//             return false;
//         }
//         SRPT::ByteVector byteData;
//         if (stream->Read(byteData)) {
//             if (byteData.empty()) {
//                 std::cout << "Received empty data" << std::endl;
//                 return false;  // Return false for empty reads
//             }
//             data = std::string(byteData.begin(), byteData.end());
//             std::cout << "Received data: " << data << std::endl;
//             return true;
//         } else {
//             std::cerr << "Failed to receive data" << std::endl;
//             return false;
//         }
//     } catch (const std::exception& e) {
//         std::cerr << "Exception during data receiving: " << e.what() << std::endl;
//         return false;
//     }
// }
