#include "../include/UDP_Sender.h"
#include "../include/utils.h"
#include <JuceHeader.h>
#include <thread>

#pragma once
#define SENDER_PORT 2468
#define LISTEN_PORT 1357

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

    void initSockets() { UDP_sender = new UDP_Sender(SENDER_PORT); }

    void destroySockets() { delete UDP_sender; }

    ~MainContentComponent() override { destroySockets(); }

private:
    // GUI related
    juce::Label titleLabel;
    juce::TextButton TestButton;
    juce::TextButton Node2Button;

    // Ethernet related
    UDP_Sender *UDP_sender{nullptr};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainContentComponent)
};
