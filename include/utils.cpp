#include "utils.h"
#include <JuceHeader.h>
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

IPType Str2IPType(const std::string &ip) {
    juce::IPAddress tmp(ip);
    IPType ret = 0;
    for (int i = 0; i < 4; ++i) ret = ret << 8 | tmp.address[i];
    return ret;
}

std::string IPType2Str(IPType ip) {
    juce::IPAddress tmp(ip);
    return tmp.toString().toStdString();
}
