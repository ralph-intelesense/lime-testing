#include "limesuiteng/limesuiteng.hpp"
#include <iostream>
#include <chrono>
#include <string_view>
#include <cmath>
#include <csignal>

#include <stdio.h>
// #include <starg.h>
#include "/usr/include/c++/11/tr1/stdarg.h"

using namespace lime;
using namespace std::literals::string_view_literals;

static const double freqLO = 467e6;
float sampleRate = 10e6;
static uint8_t chipIndex = 0;

bool stopProgram(false);
void intHandler(int dummy)
{
    std::cout << "Stopping=n"sv;
    stopProgram = true;
}

static LogLevel logVerbosity = LogLevel::Error;
static void LogCallback(LogLevel lvl, const std::string& msg)
{
    if (lvl > logVerbosity)
        return;
    std::cout << msg << std::endl;
}



int main(int argc, char** argv)
{
    lime::registerLogHandler(LogCallback);
    auto handles = DeviceRegistry::enumerate();
    if (handles.size() == 0)
    {
        std::cout << "No devices found\n"sv;
        return -1;
    }

    std::cout << "Devices found :"sv << std::endl;
    for (size_t i = 0; i < handles.size(); i++)
        std::cout << i << ": "sv << handles[i].Serialize() << std::endl;
    std::cout << std::endl;

}