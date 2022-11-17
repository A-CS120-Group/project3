#ifndef UDP_SENDER_H
#define UDP_SENDER_H

#include "utils.h"
#include <JuceHeader.h>
#include <cassert>
#include <ostream>
#include <queue>

// TODO: Node 2 and Node 3 should both receive and send

class UDP_Sender {
public:
    UDP_Sender() = delete;

    UDP_Sender(const UDP_Sender &) = delete;

    UDP_Sender(const UDP_Sender &&) = delete;

    explicit UDP_Sender(int bind_port) : port(bind_port) {
        UDP_Socket = new juce::DatagramSocket(false);
        if (!UDP_Socket->bindToPort(bind_port)) {
            std::cerr << "Port " << bind_port << " in use!" << std::endl;
            exit(bind_port);
        }
        std::cerr << "UDP_Sender Thread Start\n" << std::endl;
    }

    ~UDP_Sender() {
        UDP_Socket->shutdown();
        delete UDP_Socket;
    }

    void send(char *buffer, const std::string &ip, int target_port) {
        UDP_Socket->write(ip, target_port, buffer, 40);
    }

private:
    int port;

    juce::DatagramSocket *UDP_Socket{nullptr};
};

#endif//UDP_SENDER_H
