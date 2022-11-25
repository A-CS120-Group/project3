#ifndef ICMP_H
#define ICMP_H

#include "config.h"
#include "icmp_header.h"
#include "icmp_request_send.h"
#include "ipv4_header.h"
#include "utils.h"
#include <JuceHeader.h>

#include <utility>

class ICMP : public Thread {
public:
    ICMP() = delete;

    ICMP(const ICMP &) = delete;

    ICMP(const ICMP &&) = delete;

    explicit ICMP(std::string self, ProcessorType processFunc) : Thread("ICMP"), self_ip(std::move(self)) { fprintf(stderr, "\t\tICMP Thread Start\n"); }

    ~ICMP() override {
        this->signalThreadShouldExit();
        this->waitForThreadToExit(1000);
    }

    void run() override {
        while (!threadShouldExit()) {
            // Read from pipe
            std::ifstream pipe("./pipe");
            std::string line1, line2, line3;
            pipe >> line1 >> line2 >> line3;
            if (line1.empty() | line2.empty() | line3.empty()) continue;
            // assert line1, line2 are ip address
            if (line2 != self_ip) continue;
            // Discard other packs
            std::cerr << line1 << " " << line2 << " " << line3 << std::endl;
//            FrameType frame{Config::ICMP, line2, std::string(buffer, len)};
//            process(frame);
        }
    }

    static void send(const std::string &payload, const std::string &ip) {
        icmp_request_send(ip, payload);
        fprintf(stderr, "\t\tICMP sent\n");
    }

private:
    std::string self_ip;
};

#endif//ICMP_H
