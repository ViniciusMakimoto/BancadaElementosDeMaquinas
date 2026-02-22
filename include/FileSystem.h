#pragma once
#include <Arduino.h>
#include "Config.h"

#ifndef WOKWI_EMU
#include <LittleFS.h>
#endif

class FileSystem
{
public:
    static bool begin();
};