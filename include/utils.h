#pragma once

#include <algorithm>
#include <boost/crc.hpp>
#include <chrono>
#include <iostream>
#include <utility>
#include <vector>

#define NOT_REACHED                                                                                                                                                                \
    do { exit(123); } while (false);

using LENType = unsigned char;
using TYPEType = unsigned char;
using IPType = unsigned int;
using PORTType = unsigned short;

constexpr int LENGTH_OF_ONE_BIT = 4;
constexpr int MTU = 60;
constexpr int LENGTH_PREAMBLE = 3;
constexpr int LENGTH_LEN = sizeof(LENType);
constexpr int LENGTH_TYPE = sizeof(TYPEType);
constexpr int LENGTH_IP = sizeof(IPType);
constexpr int LENGTH_PORT = sizeof(PORTType);
constexpr int LENGTH_CRC = 4;
constexpr int MAX_LENGTH_BODY = MTU - LENGTH_PREAMBLE - LENGTH_LEN - LENGTH_TYPE - LENGTH_IP - LENGTH_PORT - LENGTH_CRC;

const std::string preamble{0x55, 0x55, 0x54};

constexpr float PREAMBLE_THRESHOLD = 0.3f;

unsigned int crc32(const char *src, size_t srcSize);

int judgeBit(float signal1, float signal2);

IPType Str2IPType(const std::string &ip);

std::string IPType2Str(IPType ip);

template<class T>
[[nodiscard]] std::string inString(T object) {
    return {(const char *) &object, sizeof(T)};
}

/* Structure of a frame
 * PREAMBLE
 * LEN      the length of BODY;
 * TYP      type of protocol;
 * IP
 * PORT
 * BODY
 * CRC
 */

class FrameType {
public:
    enum { udp = 1, tcp = 2, ping = 3, pong = 4 };
    LENType len = 0;
    TYPEType type = 0;
    IPType ip = 0;
    PORTType port = 0;
    std::string body;

    FrameType() = default;

    FrameType(TYPEType nType, IPType nIp, PORTType nPort, std::string nBody) : len((LENType) nBody.size()), type(nType), ip(nIp), port(nPort), body(std::move(nBody)) {}

    [[nodiscard]] std::string wholeString() const { return inString(len) + inString(type) + inString(ip) + inString(port) + body; }

    [[nodiscard]] unsigned int crc() const {
        auto str = wholeString();
        return crc32(str.c_str(), str.size());
    }
};

using std::chrono::steady_clock;

class MyTimer {
public:
    std::chrono::time_point<steady_clock> start;

    MyTimer() : start(steady_clock::now()) {}

    void restart() { start = steady_clock::now(); }

    [[nodiscard]] double duration() const {
        auto now = steady_clock::now();
        return std::chrono::duration<double>(now - start).count();
    }
};
