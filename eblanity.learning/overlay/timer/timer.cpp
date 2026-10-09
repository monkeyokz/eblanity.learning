#include "timer.h"

namespace overlay::timer {
namespace {
int64_t EstimateOsSleepOverhead() {
    constexpr int samples = 16;
    int64_t overheadTicks = 0;
    for (int i = 0; i < samples; ++i) {
        const int64_t before = GetTicks();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        const int64_t after = GetTicks();
        overheadTicks += (after - before) - (GetFrequency() / 1000);
    }
    return (overheadTicks / samples) + (GetFrequency() / 2000);
}

int64_t GetSleepOverhead() {
    static const int64_t overheadTicks = []() {
        HANDLE thread = GetCurrentThread();
        DWORD_PTR previousAffinity = SetThreadAffinityMask(thread, 1);
        int previousPriority = GetThreadPriority(thread);
        const int64_t result = EstimateOsSleepOverhead();
        SetThreadAffinityMask(thread, previousAffinity);
        SetThreadPriority(thread, previousPriority);
        return result;
    }();
    return overheadTicks;
}

double GetInvFrequency() {
    static const double inverseFrequency = 1000.0 / static_cast<double>(GetFrequency());
    return inverseFrequency;
}
}

void Init() { static_cast<void>(GetSleepOverhead()); }

void PrecisionSleep(double milliseconds) {
    if (milliseconds <= 0.0) return;
    const int64_t frequency = GetFrequency();
    const int64_t overheadTicks = GetSleepOverhead();
    const int64_t targetTicks = GetTicks() + static_cast<int64_t>(milliseconds * frequency * 0.001);
    const int64_t sleepThreshold = overheadTicks + frequency / 1000;
    const int64_t yieldThreshold = frequency / 4000;
    const int64_t pauseThreshold = frequency / 20000;
    while (true) {
        const int64_t remainingTicks = targetTicks - GetTicks();
        if (remainingTicks <= sleepThreshold) break;
        const int64_t sleepMs = (std::max)(int64_t(1), (remainingTicks - overheadTicks) * 1000 / frequency - 1);
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepMs));
    }
    while (targetTicks - GetTicks() > yieldThreshold) std::this_thread::yield();
    while (targetTicks - GetTicks() > pauseThreshold) {
        _mm_pause();
        _mm_pause();
        _mm_pause();
        _mm_pause();
    }
    while (GetTicks() < targetTicks) _mm_pause();
}

double GetTime() { return static_cast<double>(GetTicks()) * GetInvFrequency(); }

int64_t GetTicks() {
    LARGE_INTEGER ticks{};
    QueryPerformanceCounter(&ticks);
    return ticks.QuadPart;
}

int64_t GetFrequency() {
    static const int64_t frequency = []() {
        LARGE_INTEGER value{};
        QueryPerformanceFrequency(&value);
        return value.QuadPart;
    }();
    return frequency;
}
}
