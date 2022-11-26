#include "../include/config.h"
#include "../include/reader.h"
#include "../include/utils.h"
#include "../include/writer.h"
#include <JuceHeader.h>
#include <fstream>
#include <queue>
#include <thread>

#pragma once

class MainContentComponent : public juce::AudioAppComponent {
public:
    MainContentComponent() {

        titleLabel.setText("Node1", juce::NotificationType::dontSendNotification);
        titleLabel.setSize(160, 40);
        titleLabel.setFont(juce::Font(36, juce::Font::FontStyleFlags::bold));
        titleLabel.setJustificationType(juce::Justification(juce::Justification::Flags::centred));
        titleLabel.setCentrePosition(300, 40);
        addAndMakeVisible(titleLabel);

        Part2CK1.setButtonText("Part2CK1 UDP 1->3");
        Part2CK1.setSize(80, 40);
        Part2CK1.setCentrePosition(150, 140);
        Part2CK1.onClick = [this]() {
            // Transmission Initialization
            std::ifstream fIn("INPUT.txt");
            if (fIn.is_open()) {
                fprintf(stderr, "successfully open INPUT.txt!\n");
            } else {
                fprintf(stderr, "failed to open INPUT.txt!\n");
                return;
            }
            auto conf = GlobalConfig().get(Config::NODE3, Config::UDP);
            std::string body;
            for (char c; fIn.get(c);) {
                if (c != '\n') {
                    body.push_back(c);
                    continue;
                }
                FrameType frame{Config::UDP, Str2IPType(conf.ip), (PORTType) conf.port, body};
                writer->send(frame);
                body.clear();
            }
        };
        addAndMakeVisible(Part2CK1);

        Part3.setButtonText("Part3 Ping 1->3");
        Part3.setSize(80, 40);
        Part3.setCentrePosition(450, 140);
        Part3.onClick = [this] {
            pingFrame.type = Config::PING;
            pingFrame.ip = GlobalConfig().get(Config::NODE3, Config::UDP).ip;
            pingFrame.identifier = "a1b2c3";
            pingFrame.payload = "hello from caoster & FlashHu";
            for (pingFrame.seq = 1; pingFrame.seq <= 10; ++pingFrame.seq) {
                MyTimer timer;
                writer->send(pingFrame.toFrameType());
                while (timer.duration() < 1.0)
                    ;
            }
        };
        addAndMakeVisible(Part3);

        setSize(600, 300);
        setAudioChannels(1, 1);
    }

    ~MainContentComponent() override { shutdownAudio(); }

private:
    void initThreads() {
        auto processFunc = [this](FrameType &frame) {
            if (frame.type == Config::UDP) {
                // receive UDP
                fprintf(stderr, "receive UDP! %s:%u %s\n", IPType2Str(frame.ip).c_str(), frame.port, frame.body.c_str());
            } else if (frame.type == Config::PING) {
                // send PONG
                fprintf(stderr, "receive PING! %s %s\n", IPType2Str(frame.ip).c_str(), frame.body.c_str());
                frame.type = Config::PONG;
                writer->send(frame);
            } else if (frame.type == Config::PONG) {
                // receive PONG
                ICMPFrameType pongFrame;
                pongFrame.fromFrameType(frame);
                if (pingFrame.ip == pongFrame.ip && pingFrame.seq == pongFrame.seq) {
                    // PING and PONG match
                    fprintf(stderr, "receive PONG! %s %s\n", pongFrame.ip.c_str(), frame.body.c_str());
                }
            }
        };
        reader = new Reader(&directInput, &directInputLock, processFunc);
        reader->startThread();
        writer = new Writer(&directOutput, &directOutputLock);
    }

    void prepareToPlay([[maybe_unused]] int samplesPerBlockExpected, [[maybe_unused]] double sampleRate) override {
        initThreads();
        AudioDeviceManager::AudioDeviceSetup currentAudioSetup;
        deviceManager.getAudioDeviceSetup(currentAudioSetup);
        currentAudioSetup.bufferSize = 144;// 144 160 192
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
                buffer->clear();
                // Write if PHY layer wants
                float *writePosition = buffer->getWritePointer(channel);
                for (int i = 0; i < bufferSize; ++i) writePosition[i] = 0.0f;
                directOutputLock.enter();
                for (int i = 0; i < bufferSize; ++i) {
                    if (directOutput.empty()) continue;
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

private:
    // PING
    ICMPFrameType pingFrame;

    // AtherNet related
    Reader *reader{nullptr};
    Writer *writer{nullptr};
    std::queue<float> directInput;
    CriticalSection directInputLock;
    std::queue<float> directOutput;
    CriticalSection directOutputLock;

    // GUI related
    juce::Label titleLabel;
    juce::TextButton Part2CK1;
    juce::TextButton Part3;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainContentComponent)
};
