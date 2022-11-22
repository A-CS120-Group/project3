#include "../include/UDP.h"
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

        TestButton.setButtonText("Send");
        TestButton.setSize(80, 40);
        TestButton.setCentrePosition(150, 140);
        TestButton.onClick = [=]() {
            char here[40] = "This is a sentence, This is a sentence!";
            Config config = globalConfig.get(Config::NODE2, Config::UDP);
            for (int i = 0; i < 10; ++i) { UDP_socket->send(here, config.ip, config.port); }
        };
        addAndMakeVisible(TestButton);

        Node2Button.setButtonText("Node2");
        Node2Button.setSize(80, 40);
        Node2Button.setCentrePosition(450, 140);
        Node2Button.onClick = nullptr;
        addAndMakeVisible(Node2Button);

        setSize(600, 300);

        initSockets();
    }

    void initSockets() {
        UDP_socket = new UDP(globalConfig.get(Config::NODE3, Config::UDP).port, nullptr, nullptr);
        UDP_socket->startThread();
    }

    void destroySockets() { delete UDP_socket; }

    ~MainContentComponent() override { destroySockets(); }

private:
    // GUI related
    juce::Label titleLabel;
    juce::TextButton TestButton;
    juce::TextButton Node2Button;

    // Ethernet related
    GlobalConfig globalConfig{};
    UDP *UDP_socket{nullptr};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainContentComponent)
};
