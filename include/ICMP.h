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
            std::ifstream pipe("./py2cpp");
            std::string src_ip_addr, dst_ip_addr, icmp_type, icmp_identifier, icmp_seq, icmp_payload;
            pipe >> src_ip_addr >> dst_ip_addr >> icmp_type >> icmp_identifier >> icmp_seq >> icmp_payload;
            if (src_ip_addr.empty()) continue;
            if (dst_ip_addr != self_ip) continue;
            // Discard other packs
            std::cerr << "receive: " << src_ip_addr << dst_ip_addr << icmp_type << icmp_identifier << icmp_seq << icmp_payload;
            // type = "request"  # or reply

            // TODO: Throw into Athernet
            //            FrameType frame{Config::ICMP, line2, std::string(buffer, len)};
            //            process(frame);
        }
    }

    static void send(const std::string &type, const std::string &ip, const std::string &identifier, int seq, const std::string &icmp_payload) {
        // Write to pipe
        std::ofstream pipe("./cpp2py");
        // mode = "request"  # or reply
        // dst_ip_addr = "192.168.1.101"
        // identifier = "6566"
        // seq = "14"
        // icmp_payload = "61626364"
        pipe << type << " " << ip << " " << identifier << " " << seq << icmp_payload << std::endl;
        fprintf(stderr, "\t\tICMP sent\n");
    }

private:
    std::string self_ip;
};

#endif//ICMP_H
