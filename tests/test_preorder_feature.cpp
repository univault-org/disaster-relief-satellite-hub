#include <gtest/gtest.h>
#include "riif_ultrasonic.h"
#include <portaudio.h>
#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <fstream>

class UltrasonicKeyTest : public ::testing::Test {
protected:
    RiifUltrasonic riif;
    std::vector<float> audioBuffer;
    std::atomic<bool> isRecording{false};

    void SetUp() override {
        Pa_Initialize();
        RiifUltrasonic::Parameters params;
        params.sampleRate = 48000;
        params.f0 = 18000;
        params.df = 1000;
        params.samplesPerFrame = 480;  // 10 ms per frame
        params.preambleDuration = 24000;  // 0.5 seconds
        riif.setParameters(params);
    }

    void TearDown() override {
        Pa_Terminate();
    }

    static int paCallback(const void *inputBuffer, void *outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo* timeInfo,
                          PaStreamCallbackFlags statusFlags,
                          void *userData) {
        UltrasonicKeyTest* test = static_cast<UltrasonicKeyTest*>(userData);
        const float* in = static_cast<const float*>(inputBuffer);
        
        if (test->isRecording) {
            test->audioBuffer.insert(test->audioBuffer.end(), in, in + framesPerBuffer);
        }
        
        return paContinue;
    }

    void startRecording(double duration) {
        PaStream *stream;
        Pa_OpenDefaultStream(&stream, 1, 0, paFloat32, riif.getParameters().sampleRate,
                             256, paCallback, this);
        isRecording = true;
        Pa_StartStream(stream);
        std::this_thread::sleep_for(std::chrono::duration<double>(duration));
        isRecording = false;
        Pa_StopStream(stream);
        Pa_CloseStream(stream);
    }

    void saveAudioToFile(const std::vector<float>& audio, const std::string& filename) {
        std::ofstream file(filename, std::ios::binary);
        if (file.is_open()) {
            file.write(reinterpret_cast<const char*>(audio.data()), audio.size() * sizeof(float));
            file.close();
            std::cout << "Audio data saved to " << filename << std::endl;
        } else {
            std::cerr << "Unable to open file for writing: " << filename << std::endl;
        }
    }

    void analyzeFrequencyContent(const std::vector<float>& signal) {
        size_t fftSize = 4096;
        std::vector<float> magnitudes(fftSize / 2 + 1, 0.0f);

        for (size_t i = 0; i < signal.size() - fftSize; i += fftSize / 2) {
            std::vector<float> window(fftSize);
            for (size_t j = 0; j < fftSize; ++j) {
                window[j] = signal[i + j] * (0.54f - 0.46f * std::cos(2 * M_PI * j / (fftSize - 1))); // Hamming window
            }

            // Perform FFT (you may need to implement or use a library for this)
            // For simplicity, we'll just sum the magnitudes here
            for (size_t j = 0; j < fftSize; ++j) {
                magnitudes[j / 2] += std::abs(window[j]);
            }
        }

        // Find peak frequencies
        std::cout << "Peak frequencies:" << std::endl;
        for (size_t i = 1; i < magnitudes.size() - 1; ++i) {
            if (magnitudes[i] > magnitudes[i-1] && magnitudes[i] > magnitudes[i+1]) {
                float freq = i * riif.getParameters().sampleRate / fftSize;
                if (freq > 17000 && freq < 20000) { // Only print ultrasonic frequencies
                    std::cout << "  " << freq << " Hz" << std::endl;
                }
            }
        }
    }

    std::vector<uint8_t> receiveKey() {
        audioBuffer.clear();
        std::cout << "Starting 5-second audio recording..." << std::endl;
        startRecording(5); // Record for 5 seconds
        std::cout << "Audio recording completed. Samples recorded: " << audioBuffer.size() << std::endl;

        // Print signal statistics
        std::cout << "Signal statistics:" << std::endl;
        std::cout << "  Min: " << *std::min_element(audioBuffer.begin(), audioBuffer.end()) << std::endl;
        std::cout << "  Max: " << *std::max_element(audioBuffer.begin(), audioBuffer.end()) << std::endl;
        std::cout << "  Mean: " << std::accumulate(audioBuffer.begin(), audioBuffer.end(), 0.0) / audioBuffer.size() << std::endl;

        // Find preamble
        size_t preambleStart = findPreamble(audioBuffer);
        if (preambleStart == audioBuffer.size()) {
            std::cout << "Preamble not found" << std::endl;
            return {};
        }
        std::cout << "Preamble found at sample: " << preambleStart << std::endl;

        // Find the start marker
        size_t markerStart = findStartMarker(audioBuffer, preambleStart + riif.getParameters().preambleDuration);
        if (markerStart == audioBuffer.size()) {
            std::cout << "Start marker not found" << std::endl;
            return {};
        }
        std::cout << "Start marker found at sample: " << markerStart << std::endl;

        // Convert float samples to int16_t for decoding, starting from the marker end
        std::vector<int16_t> samples;
        size_t dataStart = markerStart + (riif.getParameters().sampleRate * 0.1); // 0.1s marker duration
        samples.reserve(audioBuffer.size() - dataStart);
        for (size_t i = dataStart; i < audioBuffer.size(); ++i) {
            samples.push_back(static_cast<int16_t>(audioBuffer[i] * 32767));
        }

        std::vector<bool> decodedBits = riif.decode(samples);
        std::cout << "Decoded " << decodedBits.size() << " bits" << std::endl;

        // Print all decoded bits
        std::cout << "Decoded bits: ";
        for (size_t i = 0; i < decodedBits.size(); ++i) {
            std::cout << (decodedBits[i] ? '1' : '0');
            if ((i + 1) % 8 == 0) std::cout << ' ';
        }
        std::cout << std::endl;

        std::vector<uint8_t> key;
        key.reserve(16);
        for (size_t i = 0; i < 128 && i < decodedBits.size(); i += 8) {
            uint8_t byte = 0;
            for (int j = 0; j < 8; ++j) {
                byte = (byte << 1) | decodedBits[i + j];
            }
            key.push_back(byte);
        }

        return key;
    }

    size_t findStartMarker(const std::vector<float>& signal, size_t startFrom) {
        auto params = riif.getParameters();
        size_t markerSize = params.sampleRate * 0.1; // 0.1 seconds marker
        float threshold = 0.02f;

        std::vector<float> markerSignal(markerSize);
        for (size_t i = 0; i < markerSize; ++i) {
            float t = static_cast<float>(i) / params.sampleRate;
            markerSignal[i] = std::sin(2 * M_PI * (params.f0 + params.df) * t);
        }

        for (size_t i = startFrom; i < signal.size() - markerSize; ++i) {
            float correlation = 0;
            for (size_t j = 0; j < markerSize; ++j) {
                correlation += signal[i + j] * markerSignal[j];
            }
            correlation /= markerSize; // Normalize correlation
            if (correlation > threshold) {
                std::cout << "Start marker found at sample " << i << " with correlation " << correlation << std::endl;
                return i;
            }
        }

        std::cout << "Start marker not found" << std::endl;
        return signal.size();
    }

    size_t findPreamble(const std::vector<float>& signal) {
        auto params = riif.getParameters();
        size_t preambleSize = params.sampleRate * 0.5; // 0.5 seconds preamble
        float threshold = 0.02f;

        std::vector<float> preambleSignal(preambleSize);
        for (size_t i = 0; i < preambleSize; ++i) {
            float t = static_cast<float>(i) / params.sampleRate;
            float preambleFreq = params.f0 + (params.df * i / preambleSize);
            preambleSignal[i] = std::sin(2 * M_PI * preambleFreq * t);
        }

        float maxCorrelation = 0;
        size_t maxCorrelationIndex = signal.size();

        for (size_t i = 0; i < signal.size() - preambleSize; ++i) {
            float correlation = 0;
            for (size_t j = 0; j < preambleSize; ++j) {
                correlation += signal[i + j] * preambleSignal[j];
            }
            correlation /= preambleSize; // Normalize correlation
            if (correlation > maxCorrelation) {
                maxCorrelation = correlation;
                maxCorrelationIndex = i;
            }
            if (correlation > threshold) {
                std::cout << "Preamble found at sample " << i << " with correlation " << correlation << std::endl;
                return i;
            }
        }

        std::cout << "Preamble not found. Max correlation: " << maxCorrelation 
                  << " at index: " << maxCorrelationIndex << std::endl;
        return signal.size();
    }
};

TEST_F(UltrasonicKeyTest, ReceiveEncryptionKey) {
    std::cout << "Press Enter when ready to receive the encryption key..." << std::endl;
    std::cin.get(); // Wait for user input

    std::vector<uint8_t> receivedKey = receiveKey();

    std::cout << "Received key size: " << receivedKey.size() << " bytes" << std::endl;
    std::cout << "Received key: ";
    for (uint8_t byte : receivedKey) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    }
    std::cout << std::dec << std::endl;

    ASSERT_EQ(receivedKey.size(), 16) << "Failed to receive a 16-byte encryption key";
}
