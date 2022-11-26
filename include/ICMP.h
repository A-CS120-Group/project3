#ifndef ICMP_H
#define ICMP_H

#include "config.h"
#include "icmp_header.h"
#include "icmp_request_send.h"
#include "ipv4_header.h"
#include "utils.h"
#include <JuceHeader.h>
#include <sstream>
#include <utility>

class ICMP : public Thread {
public:
    ICMP() = delete;

    ICMP(const ICMP &) = delete;

    ICMP(const ICMP &&) = delete;

    explicit ICMP(std::string self, ICMPProcessorType processFunc) : Thread("ICMP"), self_ip(std::move(self)), process(std::move(processFunc)) {
        fprintf(stderr, "\t\tICMP Thread Start\n");
    }

    ~ICMP() override {
        this->signalThreadShouldExit();
        this->waitForThreadToExit(1000);
    }

    void run() override {
        while (!threadShouldExit()) {
            // Read from pipe
            std::ifstream pipe("./py2cpp");
            std::string dst_ip_addr;
            ICMPFrameType frame;
            pipe >> frame.ip >> dst_ip_addr >> frame.type >> frame.identifier >> frame.seq >> frame.payload;
            if (frame.ip.empty()) continue;
            if (dst_ip_addr != self_ip) continue;
            // Discard other packs
            std::cerr << "receive: " << frame.ip << " " << dst_ip_addr << " " << frame.type << " " << frame.identifier << " " << frame.seq << " " << frame.payload << std::endl;
            process(frame);
        }
    }

    static void send(const ICMPFrameType& frame) {
        // Write to pipe
        std::ofstream pipe("./cpp2py");
        // mode = "request"  # or reply
        // dst_ip_addr = "192.168.1.101"
        // identifier = "6566"
        // seq = "14"
        // icmp_payload = "61626364"
        std::cout << frame.payload << std::endl;
        std::cout << frame.type << ' ' << frame.ip << ' ' << frame.identifier << ' ' << frame.seq << ' ' << frame.payload << std::endl;
        pipe << frame.type << ' ' << frame.ip << ' ' << frame.identifier << ' ' << frame.seq << ' ' << frame.payload << std::endl;
        fprintf(stderr, "\t\tICMP sent\n");
    }

private:
    std::string self_ip;
    ICMPProcessorType process;
};

#endif//ICMP_H
