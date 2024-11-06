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

static const double frequencyLO = 467e6;
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

    SDRDevice* device = DeviceRegistry::makeDevice(handles.at(0));
    if (!device)
    {
        std::cout << "Failed to connect to device"sv << std::endl;
        return -1;
    }
    device->SetMessageLogCallback(LogCallback);
    device->Init();

    SDRConfig config;
    for (int c = 0; c < 2; ++c)
    {
        config.channel[c].rx.enabled = true;
        config.channel[c].rx.centerFrequency = frequencyLO;
        config.channel[c].rx.sampleRate = sampleRate;
        config.channel[c].rx.oversample = 2;
        config.channel[c].rx.lpf = 0;
        config.channel[c].rx.path = 2;
        config.channel[c].rx.calibrate = false;
    }

    StreamConfig stream;
    stream.channels[TRXDir::Rx] = { 0, 1 };

    stream.format = DataFormat::F32;
    stream.linkFormat = DataFormat::I16;

    signal(SIGINT, intHandler);

    const int samplesInBuffer = 256 * 4;
    complex32f_t** rxSamples = new complex32f_t*[2];
    for (int i = 0; i < 2; ++i)
        rxSamples[i] = new complex32f_t[samplesInBuffer];

    std::cout << "Configuring device ...\n"sv;
    try
    {
        auto t1 = std::chrono::high_resolution_clock::now();
        device->Configure(config, chipIndex);
        auto t2 = std::chrono::high_resolution_clock::now();
        std::cout << "SDR configured in "sv << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << "ms\n"sv;

        device->StreamSetup(stream, chipIndex);
        device->StreamStart(chipIndex);
    }
    catch(std::runtime_error& e)
    {
        std::cerr << "Failed to configure settings: "sv << e.what() << std::endl;
        return -1;
    } catch (std::logic_error& e)
    {
        std::cout << "Failed to configure settings: "sv << e.what() << std::endl;
        return -1;
    }
    std::cout << "Stream started ...\n"sv;

    

    DeviceRegistry::freeDevice(device);
    for (int i = 0; i < 2; ++i)
        delete[] rxSamples[i];
    delete[] rxSamples;
    return 0;
}