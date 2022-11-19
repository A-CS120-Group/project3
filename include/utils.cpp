#include "utils.h"
#include <cstdio>
#include <fstream>

unsigned int crc32(const char *src, size_t srcSize) {
    boost::crc_32_type crc;
    crc.process_bytes(src, srcSize);
    return crc.checksum();
}

int judgeBit(float signal1, float signal2) {
    if (signal1 - signal2 > PREAMBLE_THRESHOLD) return 1;
    else if (signal2 - signal1 > PREAMBLE_THRESHOLD)
        return 0;
    else
        return -1;
}

std::string readConfig(int lineNum) {
    std::ifstream fIn("config.txt", std::ios::binary | std::ios::in);
    while (!fIn.is_open()) {
        fprintf(stderr, "Failed to open config.txt! Retry after 1s.\n");
        MyTimer timer;
        while (timer.duration() < 1) {}
    }
    while (lineNum > 1) {
        fIn.ignore(std::numeric_limits<int>::max(), '\n');
        --lineNum;
    }
    std::string ret;
    fIn >> ret;
    fprintf(stderr, "readConfig %s\n", ret.c_str());
    return ret;
}