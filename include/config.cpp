#include "config.h"
#include <fstream>
#include <thread>
#include <iostream>


using namespace std::chrono_literals;

Config::Config() {
    std::ifstream configFile("./config.txt", std::ios::in);
    while (!configFile.is_open()) {
        fprintf(stderr, "Failed to open config.txt! Retry after 1s.\n");
        std::this_thread::sleep_for(1000ms);
    }
    while (!configFile.eof()) {
        std::string thisLine;
        configFile >> thisLine;
        std::cout << thisLine << std::endl;
        std::cout << "-------------" << std::endl;
    }
}