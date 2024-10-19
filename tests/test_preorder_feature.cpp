#include <gtest/gtest.h>
#include <portaudio.h>
#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <algorithm>
#include <cmath>
#include <fstream>
#include "ggwave/ggwave.h"
#include <cstring>
#include <random>

class GGWaveTest : public ::testing::Test {
protected:
    std::unique_ptr<GGWave> ggwave;
    std::vector<uint8_t> buffer;
    std::mt19937 rng;
    std::uniform_real_distribution<float> dist;
    std::ofstream logFile;

    void SetUp() override {
        logFile.open("ggwave_test.log");
        logFile << "Timestamp,Event,Details" << std::endl;

        auto parameters = GGWave::getDefaultParameters();
        parameters.sampleRateInp = 48000;
        parameters.sampleRateOut = 48000;
        parameters.samplesPerFrame = 1024;
        parameters.soundMarkerThreshold = 0.01f;
        parameters.sampleFormatInp = GGWAVE_SAMPLE_FORMAT_F32;
        parameters.sampleFormatOut = GGWAVE_SAMPLE_FORMAT_F32;

        try {
            ggwave = std::make_unique<GGWave>(parameters);
            ggwave->setLogFile(nullptr);  // Disable internal logging
            logEvent("SetUp", "GGWave initialized with sample rate: " + std::to_string(parameters.sampleRateInp));
            rng.seed(std::random_device()());
            dist = std::uniform_real_distribution<float>(-1.0f, 1.0f);
        } catch (const std::exception& e) {
            logEvent("Error", "GGWave initialization failed: " + std::string(e.what()));
            throw;
        }
    }

    void TearDown() override {
        logFile.close();
    }

    void logEvent(const std::string& event, const std::string& details) {
        auto now = std::chrono::system_clock::now();
        auto now_c = std::chrono::system_clock::to_time_t(now);
        logFile << std::put_time(std::localtime(&now_c), "%F %T") << "," << event << "," << details << std::endl;
        std::cout << event << ": " << details << std::endl;
    }

    void addNoise(float level) {
        const int n = buffer.size() / sizeof(float);
        auto p = reinterpret_cast<float*>(buffer.data());
        for (int i = 0; i < n; ++i) {
            p[i] = std::max(-1.0f, std::min(1.0f, p[i] + dist(rng) * level));
        }
    }
};

TEST_F(GGWaveTest, EncodeAndDecode) {
    const std::string payload = "Hello, GGWave!";
    const auto protocolId = GGWAVE_PROTOCOL_AUDIBLE_FAST;

    ASSERT_TRUE(ggwave->init(payload.size(), payload.data(), protocolId));

    const auto nBytes = ggwave->encode();
    ASSERT_GT(nBytes, 0);

    buffer.resize(nBytes);
    std::memcpy(buffer.data(), ggwave->txWaveform(), nBytes);

    // Add some artificial noise
    addNoise(0.01f);

    ggwave->decode(buffer.data(), buffer.size());

    GGWave::TxRxData result;
    const int decoded = ggwave->rxTakeData(result);
    ASSERT_EQ(decoded, payload.size());

    std::string decodedPayload(result.begin(), result.begin() + decoded);
    ASSERT_EQ(payload, decodedPayload);
}

TEST_F(GGWaveTest, VariablePayloadLength) {
    const std::string payload = "a0Z5kR2g";
    const auto protocolId = GGWAVE_PROTOCOL_DT_FASTEST;

    for (int length = 1; length <= payload.size(); ++length) {
        SCOPED_TRACE("Payload length: " + std::to_string(length));

        ASSERT_TRUE(ggwave->init(length, payload.data(), protocolId, 25));

        const auto nBytes = ggwave->encode();
        ASSERT_GT(nBytes, 0);

        buffer.resize(nBytes);
        std::memcpy(buffer.data(), ggwave->txWaveform(), nBytes);

        // Add some artificial noise
        addNoise(0.02f);

        ggwave->decode(buffer.data(), buffer.size());

        GGWave::TxRxData result;
        const int decoded = ggwave->rxTakeData(result);
        ASSERT_EQ(decoded, length);

        std::string decodedPayload(result.begin(), result.begin() + decoded);
        ASSERT_EQ(payload.substr(0, length), decodedPayload);
    }
}

class GGWaveReceiveTest : public ::testing::Test {
protected:
    std::unique_ptr<GGWave> ggwave;
    std::vector<float> audioBuffer;
    const int sampleRate = 48000;

    void SetUp() override {
        GGWave::Parameters params = GGWave::getDefaultParameters();
        params.sampleRateInp = sampleRate;
        params.sampleRateOut = sampleRate;
        params.samplesPerFrame = 1024;
        params.soundMarkerThreshold = 0.01f;
        params.sampleFormatInp = GGWAVE_SAMPLE_FORMAT_F32;
        params.sampleFormatOut = GGWAVE_SAMPLE_FORMAT_F32;

        ggwave = std::make_unique<GGWave>(params);
        std::cout << "SetUp: GGWave initialized with sample rate: " << sampleRate << std::endl;
    }

    std::string receiveMessage(const std::vector<float>& audio) {
        std::string receivedMessage;
        const size_t samplesPerFrame = ggwave->samplesPerFrame();
        size_t offset = 0;

        while (offset < audio.size()) {
            size_t remainingSamples = audio.size() - offset;
            size_t frameSamples = std::min(samplesPerFrame, remainingSamples);

            ggwave->decode(audio.data() + offset, frameSamples * sizeof(float));
            offset += frameSamples;

            GGWave::TxRxData data;
            int n = ggwave->rxTakeData(data);
            if (n > 0) {
                receivedMessage.append(data.begin(), data.begin() + n);
                std::cout << "Received chunk: " << std::string(data.begin(), data.begin() + n) << std::endl;
            }
        }

        return receivedMessage;
    }

    // Simulate audio transmission
    std::vector<float> simulateAudioTransmission(const std::string& message) {
        ggwave->init(message.size(), message.data(), GGWAVE_PROTOCOL_AUDIBLE_FAST);
        int nSamples = ggwave->encode();
        std::vector<float> audio(nSamples);
        std::memcpy(audio.data(), ggwave->txWaveform(), nSamples * sizeof(float));
        return audio;
    }
};

TEST_F(GGWaveReceiveTest, ReceiveMessage) {
    const std::string originalMessage = "Hello, GGWave!";
    std::vector<float> simulatedAudio = simulateAudioTransmission(originalMessage);
    
    std::string receivedMessage = receiveMessage(simulatedAudio);

    ASSERT_FALSE(receivedMessage.empty()) << "Failed to receive any message";
    ASSERT_EQ(receivedMessage, originalMessage) << "Received message does not match original";
    
    std::cout << "Test: Received message: " << receivedMessage << std::endl;
}

class GGWavePWAReceiveTest : public ::testing::Test {
protected:
    std::unique_ptr<GGWave> ggwave;
    std::vector<float> audioBuffer;
    const int sampleRate = 48000;
    PaStream *stream;
    bool isListening;
    std::vector<uint8_t> receivedMessage;
    std::mutex mutex;
    std::condition_variable cv;
    bool messageReceived = false;

    void SetUp() override {
        GGWave::Parameters params = GGWave::getDefaultParameters();
        params.sampleRateInp = sampleRate;
        params.sampleRateOut = sampleRate;
        params.samplesPerFrame = 1024;
        params.soundMarkerThreshold = 0.01f;
        params.sampleFormatInp = GGWAVE_SAMPLE_FORMAT_F32;
        params.sampleFormatOut = GGWAVE_SAMPLE_FORMAT_F32;
        params.payloadLength = 17; // Expecting 16-byte encryption key + 1 marker byte

        ggwave = std::make_unique<GGWave>(params);
        std::cout << "SetUp: GGWave initialized with sample rate: " << sampleRate << std::endl;
        std::cout << "Using protocol: GGWAVE_PROTOCOL_AUDIBLE_FAST (" << GGWAVE_PROTOCOL_AUDIBLE_FAST << ")" << std::endl;

        // Initialize PortAudio
        Pa_Initialize();
        stream = nullptr;
        isListening = false;
    }

    void TearDown() override {
        if (stream) {
            Pa_CloseStream(stream);
        }
        Pa_Terminate();
    }

    // This callback is similar to the audio callback in ggwave-rx.cpp
    static int audioCallback(const void *inputBuffer, void *outputBuffer,
                             unsigned long framesPerBuffer,
                             const PaStreamCallbackTimeInfo* timeInfo,
                             PaStreamCallbackFlags statusFlags,
                             void *userData) {
        GGWavePWAReceiveTest* test = static_cast<GGWavePWAReceiveTest*>(userData);
        const float* in = static_cast<const float*>(inputBuffer);
        
        if (test->isListening) {
            test->processAudioChunk(in, framesPerBuffer);
        }
        
        return paContinue;
    }

    // This method is similar to the main loop in ggwave-rx.cpp
    void processAudioChunk(const float* chunk, unsigned long framesPerBuffer) {
        std::lock_guard<std::mutex> lock(mutex);
        
        float signalStrength = calculateSignalStrength(chunk, framesPerBuffer);
        std::cout << "Signal strength: " << signalStrength << " dB" << std::endl;
        
        // Process the audio data
        int result = ggwave->decode(chunk, framesPerBuffer * sizeof(float));
        std::cout << "Decode result: " << result << std::endl;

        // Check for received data
        GGWave::TxRxData data;
        int n = ggwave->rxTakeData(data);
        if (n > 0) {
            std::cout << "Received data, size: " << n << std::endl;
            if (n == 17 && data[0] == 77) { // Check for marker byte and correct length
                receivedMessage.assign(data.begin() + 1, data.end());
                std::cout << "Decoded message, size: " << (n - 1) << std::endl;
                std::cout << "Using protocol: GGWAVE_PROTOCOL_AUDIBLE_FAST (" << GGWAVE_PROTOCOL_AUDIBLE_FAST << ")" << std::endl;
                messageReceived = true;
                cv.notify_one();
            }
        }
    }

    bool startListening(int durationSeconds) {
        PaStreamParameters inputParameters;
        inputParameters.device = Pa_GetDefaultInputDevice();
        if (inputParameters.device == paNoDevice) {
            std::cerr << "Error: No default input device." << std::endl;
            return false;
        }
        inputParameters.channelCount = 1;
        inputParameters.sampleFormat = paFloat32;
        inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
        inputParameters.hostApiSpecificStreamInfo = nullptr;

        PaError err = Pa_OpenStream(&stream, &inputParameters, nullptr, sampleRate,
                                    1024, paClipOff, audioCallback, this);
        if (err != paNoError) {
            std::cerr << "Error: Failed to open PortAudio stream. " << Pa_GetErrorText(err) << std::endl;
            return false;
        }

        err = Pa_StartStream(stream);
        if (err != paNoError) {
            std::cerr << "Error: Failed to start PortAudio stream. " << Pa_GetErrorText(err) << std::endl;
            return false;
        }

        isListening = true;
        std::cout << "Started listening for " << durationSeconds << " seconds..." << std::endl;

        // Wait for the message or timeout
        {
            std::unique_lock<std::mutex> lock(mutex);
            if (cv.wait_for(lock, std::chrono::seconds(durationSeconds), [this] { return messageReceived; })) {
                std::cout << "Message received before timeout." << std::endl;
            } else {
                std::cout << "Timeout reached without receiving a message." << std::endl;
            }
        }

        isListening = false;

        err = Pa_StopStream(stream);
        if (err != paNoError) {
            std::cerr << "Error: Failed to stop PortAudio stream. " << Pa_GetErrorText(err) << std::endl;
            return false;
        }

        return true;
    }

        float calculateSignalStrength(const float* buffer, unsigned long size) {
        float sum = 0.0f;
        for (unsigned long i = 0; i < size; ++i) {
            sum += buffer[i] * buffer[i];
        }
        float rms = std::sqrt(sum / size);
        return 20 * std::log10(rms);
    }
};

TEST_F(GGWavePWAReceiveTest, ReceiveFromPWA) {
    std::cout << "Please start the PWA transmission now." << std::endl;
    ASSERT_TRUE(startListening(60)); // Listen for 60 seconds instead of 30

    std::cout << "Received message size: " << receivedMessage.size() << std::endl;

    if (receivedMessage.empty()) {
        std::cout << "No message received." << std::endl;
    } else {
        std::cout << "Received message: ";
        for (uint8_t c : receivedMessage) {
            std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c);
        }
        std::cout << std::endl;
    }

    ASSERT_FALSE(receivedMessage.empty()) << "Failed to receive any message from PWA";
    ASSERT_EQ(receivedMessage.size(), 16) << "Received message is not a 16-byte encryption key";
}





