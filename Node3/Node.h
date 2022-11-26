#include "../include/UDP.h"
#include "../include/ICMP.h"
#include "../include/config.h"
#include "../include/utils.h"
#include <JuceHeader.h>
#include <thread>

#pragma once

class MainContentComponent : public Component {
public:
    MainContentComponent() {
        titleLabel.setText("Node3", juce::NotificationType::dontSendNotification);
        titleLabel.setSize(160, 40);
        titleLabel.setFont(juce::Font(36, juce::Font::FontStyleFlags::bold));
        titleLabel.setJustificationType(juce::Justification(juce::Justification::Flags::centred));
        titleLabel.setCentrePosition(300, 40);
        addAndMakeVisible(titleLabel);

        Part2CK2.setButtonText("Part2CK2 UDP 3->1");
        Part2CK2.setSize(80, 40);
        Part2CK2.setCentrePosition(150, 140);
        Part2CK2.onClick = [this]() {
            // Transmission Initialization
            std::ifstream fIn("INPUT.txt");
            if (fIn.is_open()) {
                fprintf(stderr, "successfully open INPUT.txt!\n");
            } else {
                fprintf(stderr, "failed to open INPUT.txt!\n");
                return;
            }
            auto conf = GlobalConfig().get(Config::NODE1, Config::UDP);
            std::string body;
            for (char c; fIn.get(c);) {
                if (c != '\n') {
                    body.push_back(c);
                    continue;
                }
                UDP_socket->send(body, conf.ip, conf.port);
                body.clear();
            }
        };
        addAndMakeVisible(Part2CK2);

        Node2Button.setButtonText("---");
        Node2Button.setSize(80, 40);
        Node2Button.setCentrePosition(450, 140);
        Node2Button.onClick = nullptr;
        addAndMakeVisible(Node2Button);

        setSize(600, 300);

        initSockets();
    }

    void initSockets() {
        auto processUDP = [](FrameType &frame) {
            if (frame.type == Config::UDP) {
                // receive UDP
                fprintf(stderr, "receive UDP! %s:%u %s\n", IPType2Str(frame.ip).c_str(), frame.port, frame.body.c_str());
            }
        };
        UDP_socket = new UDP(globalConfig.get(Config::NODE3, Config::UDP).port, processUDP);
        UDP_socket->startThread();

        auto processICMP = [](ICMPFrameType &frame) {
            if (frame.type == Config::PING) {
                // send PONG
                fprintf(stderr, "receive PING! %s %s\n", frame.ip.c_str(), frame.payload.c_str());
                frame.type = Config::PONG;
                ICMP::send(frame);
            } else if (frame.type == Config::PONG) {
                // receive PONG
                fprintf(stderr, "receive PONG! %s %s\n", frame.ip.c_str(), frame.payload.c_str());
            }
        };
        ICMP_socket = new ICMP(globalConfig.get(Config::NODE3, Config::UDP).ip, processICMP);
        ICMP_socket->startThread();
    }

    void destroySockets() { delete UDP_socket; }

    ~MainContentComponent() override { destroySockets(); }

private:
    // GUI related
    juce::Label titleLabel;
    juce::TextButton Part2CK2;
    juce::TextButton Node2Button;

    // Ethernet related
    GlobalConfig globalConfig{};
    UDP *UDP_socket{nullptr};
    ICMP *ICMP_socket{nullptr};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainContentComponent)
};
