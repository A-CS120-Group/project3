#ifndef UDP_LISTENER_H
#define UDP_LISTENER_H

#include "utils.h"
#include <JuceHeader.h>
#include <cassert>
#include <ostream>
#include <queue>

#define LISTEN_PORT 1357

// TODO: Node 2 and Node 3 should both receive and send

class UDP_Listener : public Thread {
public:
    UDP_Listener() = delete;

    UDP_Listener(const UDP_Listener &) = delete;

    UDP_Listener(const UDP_Listener &&) = delete;

    explicit UDP_Listener(int listen_port, std::queue<FrameType> *bufferOut, CriticalSection *lockOutput)
            : Thread("UDP_Listener"), output(bufferOut), protectOutput(lockOutput), port(listen_port) {
        UDP_Socket = new juce::DatagramSocket(false);
        if (!UDP_Socket->bindToPort(listen_port)) {
            std::cerr << "Port " << listen_port << " in use!" << std::endl;
            exit(listen_port);
        }
        std::cerr << "UDP_Listener Thread Start\n" << std::endl;
    }

    ~UDP_Listener() override {
        this->signalThreadShouldExit();
        this->waitForThreadToExit(1000);
        UDP_Socket->shutdown();
        delete UDP_Socket;
    }

    void run() override {
        // TODO: temporarily unused
//        assert(output != nullptr);
//        assert(protectOutput != nullptr);
        while (!threadShouldExit()) {
            char buffer[40];
            std::cout << UDP_Socket->read(buffer, 40, true) << std::endl;
            std::cout << buffer << std::endl;
        }
    }

private:
    std::queue<FrameType> *output{nullptr};
    CriticalSection *protectOutput;
    int port;

    juce::DatagramSocket *UDP_Socket{nullptr};
};

#endif//UDP_LISTENER_H
