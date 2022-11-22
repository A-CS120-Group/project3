#ifndef UDP_H
#define UDP_H

#include "utils.h"
#include <JuceHeader.h>
#include <cassert>
#include <ostream>
#include <queue>

class UDP : public Thread {
public:
    UDP() = delete;

    UDP(const UDP &) = delete;

    UDP(const UDP &&) = delete;

    explicit UDP(int listen_port, std::queue<FrameType> *bufferOut, CriticalSection *lockOutput) : Thread("UDP"), output(bufferOut), protectOutput(lockOutput), port(listen_port) {
        UDP_Socket = new juce::DatagramSocket(false);
        if (!UDP_Socket->bindToPort(listen_port)) {
            std::cerr << "Port " << listen_port << " in use!" << std::endl;
            exit(listen_port);
        }
        std::cerr << "UDP Thread Start\n" << std::endl;
    }

    ~UDP() override {
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
            char buffer[50];
            juce::String sender_ip;
            int sender_port;
            UDP_Socket->waitUntilReady(true, 10000);
            int len = UDP_Socket->read(buffer, 41, false, sender_ip, sender_port);
            if (len == 0) continue;
            buffer[len] = 0;
            std::cout << "Pack from " << sender_ip << ":" << sender_port << " with length " << len << "(bytes) and content: " << buffer << std::endl;
        }
    }

    void send(const std::string &buffer, const std::string &ip, int target_port) {
        UDP_Socket->waitUntilReady(false, 10000);
        UDP_Socket->write(ip, target_port, buffer.c_str(), static_cast<int>(buffer.size()));
    }

private:
    std::queue<FrameType> *output{nullptr};
    CriticalSection *protectOutput;
    int port;

    juce::DatagramSocket *UDP_Socket{nullptr};
};

#endif//UDP_H
