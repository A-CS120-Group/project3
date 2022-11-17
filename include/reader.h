#ifndef READER_H
#define READER_H

#include "utils.h"
#include <JuceHeader.h>
#include <cassert>
#include <ostream>
#include <queue>

class Reader : public Thread {
public:
    Reader() = delete;

    Reader(const Reader &) = delete;

    Reader(const Reader &&) = delete;

    explicit Reader(std::queue<float> *bufferIn, CriticalSection *lockInput, std::queue<FrameType> *bufferOut,
                    CriticalSection *lockOutput)
            : Thread("Reader"), input(bufferIn), output(bufferOut), protectInput(lockInput),
              protectOutput(lockOutput) {
        fprintf(stderr, "    Reader Thread Start\n");
    }

    ~Reader() override { this->signalThreadShouldExit(); }

    char readByte() {
        float buffer[LENGTH_OF_ONE_BIT];
        char byte = 0;
        int bufferPos = 0, bitPos = 0;
        while (!threadShouldExit()) {
            protectInput->enter();
            if (input->empty()) {
                protectInput->exit();
                continue;
            }
            buffer[bufferPos] = input->front();
            input->pop();
            protectInput->exit();
            if (++bufferPos == LENGTH_OF_ONE_BIT) {
                int bit = judgeBit(buffer[0], buffer[2]);
                if (bit == -1) { // shift by one sample
                    for (int i = 1; i < LENGTH_OF_ONE_BIT; ++i)
                        buffer[i - 1] = buffer[i];
                    --bufferPos;
                    continue;
                }
                bufferPos = 0;
                byte = (char) (byte | (bit << bitPos));
                if (++bitPos == 8) break;
            }
        }
        return byte;
    }

    template<class T>
    void readObject(T &object) {
        for (size_t i = 0; i < sizeof(object); ++i)
            ((char *) &object)[i] = readByte();
    }

    void waitForPreamble() {
        auto sync = std::deque<float>(LENGTH_PREAMBLE * 8 * LENGTH_OF_ONE_BIT, 0);
        while (!threadShouldExit()) {
            protectInput->enter();
            if (input->empty()) {
                protectInput->exit();
                continue;
            }
            if (sync.front() > NOISY_THRESHOLD) {
                std::cout << "";
            }
            sync.pop_front();
            sync.push_back(input->front());
            input->pop();
            protectInput->exit();
            bool isPreamble = true;
            for (unsigned i = 0; isPreamble && i < 8 * LENGTH_PREAMBLE; ++i) {
                isPreamble = (preamble[i / 8] >> (i % 8) & 1) ==
                             judgeBit(sync[i * LENGTH_OF_ONE_BIT], sync[i * LENGTH_OF_ONE_BIT + 2]);
                if (i > 10) {
                    std::cout << "";
                }
            }
            if (isPreamble)
                return;
        }
        fprintf(stderr, "exit\n");
    }

    void run() override {
        assert(input != nullptr);
        assert(output != nullptr);
        assert(protectInput != nullptr);
        assert(protectOutput != nullptr);
        while (!threadShouldExit()) {
            // wait for PREAMBLE
            waitForPreamble();
            FrameType frame;
            // read LEN, SEQ
            readObject(frame.len);
            readObject(frame.seq);
            if (frame.len > MAX_LENGTH_BODY) {
                // Too long! There must be some errors.
                fprintf(stderr, "\tDiscarded due to wrong length. len = %u, seq = %d\n", frame.len, frame.seq);
                continue;
            }
            // read BODY
            for (int i = 0; i < frame.len; ++i) { frame.body[i] = readByte(); }
            // read CRC
            unsigned int crcRead;
            readObject(crcRead);
            if (crcRead != frame.crc()) {
                fprintf(stderr, "\tDiscarded due to failing CRC check. len = %u, seq = %d\n", frame.len,
                        frame.seq);
                continue;
            }
            protectOutput->enter();
            output->push(frame);
            protectOutput->exit();
            fprintf(stderr, "\tSUCCEED! len = %u, seq = %d\n", frame.len, frame.seq);
        }
    }

private:
    std::queue<float> *input{nullptr};
    std::queue<FrameType> *output{nullptr};
    CriticalSection *protectInput;
    CriticalSection *protectOutput;
};

#endif//READER_H
