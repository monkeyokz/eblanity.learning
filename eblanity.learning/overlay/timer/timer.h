#pragma once
#include "../../include.h"
#include <chrono>

namespace overlay::timer {
void PrecisionSleep(double milliseconds);
void Init();
double GetTime();
int64_t GetTicks();
int64_t GetFrequency();
}
