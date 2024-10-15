#include <gtest/gtest.h>
#include "riif_ultrasonic.h"
#include <portaudio.h>
#include <vector>
#include <string>
#include <iostream>
#include <chrono>
#include <thread>
#include <atomic>

class PreorderUltrasonicTest : public ::testing::Test {
protected:
    RiifUltrasonic riif;
    std::vector<float> audioBuffer;
    std::atomic<bool> isRecording{false};

    void SetUp() override {
        try {
            RiifUltrasonic::Parameters params;
            params.sampleRate = 48000;
            params.f0 = 15000.0f;
            params.df = 1000.0f;
            params.samplesPerFrame = 480;
            params.rsMsgLength = 223;
            params.rsEccLength = 32;
            riif.setParameters(params);

            PaError err = Pa_Initialize();
            if (err != paNoError) {
                throw std::runtime_error(std::string("PortAudio initialization failed: ") + Pa_GetErrorText(err));
            }
        } catch (const std::exception& e) {
            FAIL() << "SetUp failed: " << e.what();
        }
    }

    void TearDown() override {
        Pa_Terminate();
    }

    static int paCallback(const void *inputBuffer, void *outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo* timeInfo,
                          PaStreamCallbackFlags statusFlags,
                          void *userData) {
        PreorderUltrasonicTest* test = static_cast<PreorderUltrasonicTest*>(userData);
        const float* in = static_cast<const float*>(inputBuffer);
        
        if (test->isRecording) {
            test->audioBuffer.insert(test->audioBuffer.end(), in, in + framesPerBuffer);
        }
        
        return paContinue;
    }

    public:  // Add this line to make the following methods public
    void startRecording(double duration) {
        PaStream *stream;
        PaError err;

        err = Pa_OpenDefaultStream(&stream, 1, 0, paFloat32, riif.getParameters().sampleRate,
                                   256, paCallback, this);
        if (err != paNoError) {
            throw std::runtime_error(std::string("PortAudio stream open failed: ") + Pa_GetErrorText(err));
        }

        isRecording = true;
        err = Pa_StartStream(stream);
        if (err != paNoError) {
            throw std::runtime_error(std::string("PortAudio stream start failed: ") + Pa_GetErrorText(err));
        }

        std::this_thread::sleep_for(std::chrono::duration<double>(duration));

        isRecording = false;
        err = Pa_StopStream(stream);
        if (err != paNoError) {
            throw std::runtime_error(std::string("PortAudio stream stop failed: ") + Pa_GetErrorText(err));
        }

        err = Pa_CloseStream(stream);
        if (err != paNoError) {
            throw std::runtime_error(std::string("PortAudio stream close failed: ") + Pa_GetErrorText(err));
        }
    }

    std::string bitsToString(const std::vector<bool>& bits) {
        std::string str;
        for (size_t i = 0; i < bits.size(); i += 8) {
            char c = 0;
            for (size_t j = 0; j < 8 && i + j < bits.size(); ++j) {
                c |= (bits[i + j] ? 1 : 0) << j;
            }
            str += c;
        }
        return str;
    }
};

TEST_F(PreorderUltrasonicTest, ReceiveUltrasonicSignal) {
    std::cout << "Starting ultrasonic signal reception test." << std::endl;

    try {
        const int recordingDuration = 30; // 30 seconds
        std::atomic<bool> signalReceived{false};
        std::string receivedMessage;

        // Start recording audio
        std::thread recordThread([this, recordingDuration, &signalReceived, &receivedMessage]() {
            PaStream *stream;
            PaError err;

            err = Pa_OpenDefaultStream(&stream, 1, 0, paFloat32, riif.getParameters().sampleRate,
                                       256, paCallback, this);
            if (err != paNoError) {
                throw std::runtime_error(std::string("PortAudio stream open failed: ") + Pa_GetErrorText(err));
            }

            isRecording = true;
            err = Pa_StartStream(stream);
            if (err != paNoError) {
                throw std::runtime_error(std::string("PortAudio stream start failed: ") + Pa_GetErrorText(err));
            }

            std::cout << "Recording started. Please play the ultrasonic signal from the PWA within 30 seconds." << std::endl;

            for (int i = 0; i < recordingDuration; ++i) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                std::cout << "Listening... " << (i + 1) << " seconds elapsed." << std::endl;
                
                if (audioBuffer.size() > riif.getParameters().sampleRate) {
                    std::vector<int16_t> tempAudioData(audioBuffer.size());
                    for (size_t j = 0; j < audioBuffer.size(); ++j) {
                        tempAudioData[j] = static_cast<int16_t>(audioBuffer[j] * 32767.0f);
                    }
                    std::vector<bool> tempDecodedData = riif.decode(tempAudioData);
                    std::string tempMessage = bitsToString(tempDecodedData);
                    
                    if (tempMessage.substr(0, 5) == "TEST:") {
                        std::cout << "Valid signal detected!" << std::endl;
                        signalReceived = true;
                        receivedMessage = tempMessage;
                        break;
                    }
                }
            }

            isRecording = false;
            err = Pa_StopStream(stream);
            if (err != paNoError) {
                throw std::runtime_error(std::string("PortAudio stream stop failed: ") + Pa_GetErrorText(err));
            }

            err = Pa_CloseStream(stream);
            if (err != paNoError) {
                throw std::runtime_error(std::string("PortAudio stream close failed: ") + Pa_GetErrorText(err));
            }
        });

        // Wait for recording to finish
        recordThread.join();

        std::cout << "Recording finished. Captured " << audioBuffer.size() << " samples." << std::endl;

        if (!signalReceived) {
            FAIL() << "No valid ultrasonic signal detected within 30 seconds.";
        }

        std::cout << "Received message: " << receivedMessage << std::endl;

        // Verify the received message
        ASSERT_TRUE(receivedMessage.substr(0, 5) == "TEST:") << "Received message does not start with 'TEST:'";
        ASSERT_FALSE(receivedMessage.substr(5).empty()) << "No order data in the received message";

        std::string receivedOrder = receivedMessage.substr(5);
        std::cout << "Received order: " << receivedOrder << std::endl;

        // You can add more specific assertions here based on the expected format of your order data
    } catch (const std::exception& e) {
        FAIL() << "Exception occurred: " << e.what();
    }
}
