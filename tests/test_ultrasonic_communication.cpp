#include <gtest/gtest.h>
#include "riif_ultrasonic.h"
#include <vector>
#include <cstdint>
#include <string>
#include <iostream>
#include <random>
#include <stdexcept>
#include <chrono>
#include <iomanip>  // For std::setw and std::setprecision

class UltrasonicTest : public ::testing::Test {
protected:
    std::chrono::high_resolution_clock::time_point setup_start;
    std::chrono::high_resolution_clock::time_point teardown_start;

    void SetUp() override {
        setup_start = std::chrono::high_resolution_clock::now();
        try {
            std::cout << "Starting SetUp..." << std::endl;
            RiifUltrasonic::Parameters params;
            params.sampleRate = 48000;
            params.f0 = 15000.0f;
            params.df = 1000.0f;
            params.samplesPerFrame = 480;  // 10 ms per bit for faster transmission
            params.rsMsgLength = 223;  // Add this line
            params.rsEccLength = 32;   // Add this line
            std::cout << "Parameters initialized." << std::endl;
            
            std::cout << "About to create RiifUltrasonic object..." << std::endl;
            riif = RiifUltrasonic();
            std::cout << "RiifUltrasonic object created." << std::endl;
            
            std::cout << "About to call setParameters..." << std::endl;
            riif.setParameters(params);
            std::cout << "SetParameters completed." << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Exception in SetUp: " << e.what() << std::endl;
            FAIL() << "SetUp failed: " << e.what();
        }
        auto setup_end = std::chrono::high_resolution_clock::now();
        auto setup_duration = std::chrono::duration_cast<std::chrono::milliseconds>(setup_end - setup_start);
        std::cout << "SetUp completed in " << setup_duration.count() << " milliseconds" << std::endl;
    }

    void TearDown() override {
        teardown_start = std::chrono::high_resolution_clock::now();
        // Add any teardown code here if needed
    }

    ~UltrasonicTest() {
        auto teardown_end = std::chrono::high_resolution_clock::now();
        auto teardown_duration = std::chrono::duration_cast<std::chrono::milliseconds>(teardown_end - teardown_start);
        std::cout << "TearDown completed in " << teardown_duration.count() << " milliseconds" << std::endl;
    }

    RiifUltrasonic riif;

    // Helper function to measure and print duration
    template<typename Func>
    long long measureAndPrint(const std::string& stepName, Func func) {
        auto start = std::chrono::high_resolution_clock::now();
        func();
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        std::cout << std::setw(30) << std::left << stepName 
                  << ": " << std::setw(10) << std::right << duration.count() 
                  << " microseconds" << std::endl;
        return duration.count();
    }
};

TEST_F(UltrasonicTest, KeyExchangeTest) {
    std::cout << "Entered KeyExchangeTest function." << std::endl;

    std::vector<bool> key;
    std::vector<int16_t> audio_samples;
    std::vector<bool> decoded_key;

    auto total_start = std::chrono::high_resolution_clock::now();

    // Key Generation
    auto key_gen_start = std::chrono::high_resolution_clock::now();
    key.resize(128);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 1);
    for (int i = 0; i < 128; ++i) {
        key[i] = dis(gen) == 1;
    }
    auto key_gen_end = std::chrono::high_resolution_clock::now();
    auto key_gen_duration = std::chrono::duration_cast<std::chrono::microseconds>(key_gen_end - key_gen_start);
    std::cout << "Key Generation time: " << key_gen_duration.count() << " microseconds" << std::endl;

    std::cout << "First 32 bits of original key: ";
    for (int i = 0; i < 32 && i < key.size(); ++i) {
        std::cout << (key[i] ? '1' : '0');
    }
    std::cout << "..." << std::endl;

    // Audio Signal Generation
    auto audio_gen_start = std::chrono::high_resolution_clock::now();
    for (bool bit : key) {
        float frequency = bit ? riif.getParameters().f0 + riif.getParameters().df : riif.getParameters().f0;
        for (int i = 0; i < riif.getParameters().samplesPerFrame; ++i) {
            float t = static_cast<float>(i) / riif.getParameters().sampleRate;
            float sample = std::sin(2 * M_PI * frequency * t);
            audio_samples.push_back(static_cast<int16_t>(sample * 32767));
        }
    }
    auto audio_gen_end = std::chrono::high_resolution_clock::now();
    auto audio_gen_duration = std::chrono::duration_cast<std::chrono::microseconds>(audio_gen_end - audio_gen_start);
    std::cout << "Audio Signal Generation time: " << audio_gen_duration.count() << " microseconds" << std::endl;

    std::cout << "Generated audio signal with " << audio_samples.size() << " samples" << std::endl;

    // Audio Signal Decoding
    auto decode_start = std::chrono::high_resolution_clock::now();
    decoded_key = riif.decode(audio_samples);
    auto decode_end = std::chrono::high_resolution_clock::now();
    auto decode_duration = std::chrono::duration_cast<std::chrono::microseconds>(decode_end - decode_start);
    std::cout << "Audio Signal Decoding time: " << decode_duration.count() << " microseconds" << std::endl;

    auto total_end = std::chrono::high_resolution_clock::now();
    auto total_duration = std::chrono::duration_cast<std::chrono::microseconds>(total_end - total_start);

    std::cout << "First 32 bits of decoded key: ";
    for (int i = 0; i < 32 && i < decoded_key.size(); ++i) {
        std::cout << (decoded_key[i] ? '1' : '0');
    }
    std::cout << "..." << std::endl;

    // Compare original and decoded keys
    EXPECT_EQ(key.size(), decoded_key.size()) << "Mismatch in number of decoded bits";
    int mismatches = 0;
    for (size_t i = 0; i < key.size() && i < decoded_key.size(); ++i) {
        if (key[i] != decoded_key[i]) {
            mismatches++;
        }
    }
    
    double bit_error_rate = static_cast<double>(mismatches) / key.size();
    std::cout << "Bit Error Rate: " << (bit_error_rate * 100) << "%" << std::endl;
    EXPECT_LT(bit_error_rate, 0.05) << "Bit error rate too high";

    std::cout << "Total Key Exchange time: " << total_duration.count() << " microseconds" << std::endl;
    std::cout << "Breakdown:" << std::endl;
    std::cout << "  Key Generation: " << key_gen_duration.count() << " microseconds (" 
              << (100.0 * key_gen_duration.count() / total_duration.count()) << "%)" << std::endl;
    std::cout << "  Audio Generation: " << audio_gen_duration.count() << " microseconds (" 
              << (100.0 * audio_gen_duration.count() / total_duration.count()) << "%)" << std::endl;
    std::cout << "  Audio Decoding: " << decode_duration.count() << " microseconds (" 
              << (100.0 * decode_duration.count() / total_duration.count()) << "%)" << std::endl;
}
