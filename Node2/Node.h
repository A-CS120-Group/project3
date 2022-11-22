#include "../include/UDP.h"
#include "../include/config.h"
#include "../include/reader.h"
#include "../include/utils.h"
#include "../include/writer.h"
#include <JuceHeader.h>
#include <fstream>
#include <map>
#include <queue>
#include <thread>
#include <vector>

#pragma once

class MainContentComponent : public juce::AudioAppComponent {
public:
    MainContentComponent() {
        titleLabel.setText("Node2", juce::NotificationType::dontSendNotification);
        titleLabel.setSize(160, 40);
        titleLabel.setFont(juce::Font(36, juce::Font::FontStyleFlags::bold));
        titleLabel.setJustificationType(juce::Justification(juce::Justification::Flags::centred));
        titleLabel.setCentrePosition(300, 40);
        addAndMakeVisible(titleLabel);

        TestButton.setButtonText("Test");
        TestButton.setSize(80, 40);
        TestButton.setCentrePosition(150, 140);
        TestButton.onClick = [=]() {
            std::string here = "This is a sentence, This is a sentence!";
            Config config = globalConfig.get(Config::NODE3, Config::UDP);
            for (int i = 0; i < 10; ++i) { UDP_socket->send(here, config.ip, config.port); }
        };
        addAndMakeVisible(TestButton);

        Node2Button.setButtonText("Node2");
        Node2Button.setSize(80, 40);
        Node2Button.setCentrePosition(450, 140);
        Node2Button.onClick = nullptr;
        addAndMakeVisible(Node2Button);

        setSize(600, 300);
        setAudioChannels(1, 1);

        initSockets();
    }

    ~MainContentComponent() override {
        shutdownAudio();
        destroySockets();
    }

private:
    void initThreads() {
        reader = new Reader(&directInput, &directInputLock, &binaryInput, &binaryInputLock);
        reader->startThread();
        writer = new Writer(&directOutput, &directOutputLock, &quiet);
    }

    void initSockets() {
        UDP_socket = new UDP(globalConfig.get(Config::NODE3, Config::UDP).port, nullptr, nullptr);
        UDP_socket->startThread();
    }

    void prepareToPlay([[maybe_unused]] int samplesPerBlockExpected, [[maybe_unused]] double sampleRate) override {
        initThreads();
        AudioDeviceManager::AudioDeviceSetup currentAudioSetup;
        deviceManager.getAudioDeviceSetup(currentAudioSetup);
        currentAudioSetup.bufferSize = 144;// 144 160 192
        // String ret = deviceManager.setAudioDeviceSetup(currentAudioSetup, true);
        fprintf(stderr, "Main Thread Start\n");
    }

    void getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill) override {
        auto *device = deviceManager.getCurrentAudioDevice();
        auto activeInputChannels = device->getActiveInputChannels();
        auto activeOutputChannels = device->getActiveOutputChannels();
        auto maxInputChannels = activeInputChannels.getHighestBit() + 1;
        auto maxOutputChannels = activeOutputChannels.getHighestBit() + 1;
        auto buffer = bufferToFill.buffer;
        auto bufferSize = buffer->getNumSamples();
        for (auto channel = 0; channel < maxOutputChannels; ++channel) {
            if ((!activeInputChannels[channel] || !activeOutputChannels[channel]) || maxInputChannels == 0) {
                bufferToFill.buffer->clear(channel, bufferToFill.startSample, bufferToFill.numSamples);
            } else {
                // Read in PHY layer
                const float *data = buffer->getReadPointer(channel);
                directInputLock.enter();
                for (int i = 0; i < bufferSize; ++i) { directInput.push(data[i]); }
                directInputLock.exit();
                // listen if the channel is quiet
                bool nowQuiet = true;
                for (int i = bufferSize - LENGTH_PREAMBLE * LENGTH_OF_ONE_BIT; i < bufferSize; ++i)
                    if (fabs(data[i]) > NOISY_THRESHOLD) {
                        nowQuiet = false;
                        //                        fprintf(stderr, "\t\tNoisy Now!!!!\n");
                        //                        std::ofstream logOut("log.out", std::ios::app);
                        //                        for (int j = 0; j < bufferSize; ++j)logOut << (int) (data[j] * 100) << ' ';
                        //                        logOut << "\n";
                        //                        logOut.close();
                        break;
                    }
                quiet.set(nowQuiet);
                buffer->clear();
                // Write if PHY layer wants
                float *writePosition = buffer->getWritePointer(channel);
                for (int i = 0; i < bufferSize; ++i) writePosition[i] = 0.0f;
                //                constexpr int W = 2;
                //                constexpr int bits[16] = {1,1,1,1,0,1,1,1,
                //                                          1,1,1,1,0,1,1,1};
                //                for (int i = 0; i < 16; ++i) {
                //                    writePosition[4 * i + 0] = writePosition[4 * i + 1] = bits[i] ? 1.0f : -1.0f;
                //                    writePosition[4 * i + 2] = writePosition[4 * i + 3] = bits[i] ? -1.0f : 1.0f;
                //                }
                directOutputLock.enter();
                for (int i = 0; i < bufferSize; ++i) {
                    if (directOutput.empty()) {
                        directOutputLock.exit();
                        directOutputLock.enter();
                        continue;
                    }
                    writePosition[i] = directOutput.front();
                    directOutput.pop();
                }
                directOutputLock.exit();
            }
        }
    }

    void releaseResources() override {
        delete reader;
        delete writer;
    }

    void destroySockets() { delete UDP_socket; }

private:
    // Process Input
    Reader *reader{nullptr};
    std::queue<float> directInput;
    CriticalSection directInputLock;
    std::queue<FrameType> binaryInput;
    CriticalSection binaryInputLock;

    // Process Output
    Writer *writer{nullptr};
    std::queue<float> directOutput;
    CriticalSection directOutputLock;
    Atomic<bool> quiet = false;

    // GUI related
    juce::Label titleLabel;
    juce::TextButton TestButton;
    juce::TextButton Node2Button;

    // Ethernet related
    GlobalConfig globalConfig{};
    UDP *UDP_socket{nullptr};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainContentComponent)
};
