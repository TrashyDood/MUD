#pragma once

#include <chrono>

using namespace std::chrono_literals;
using Clock = std::chrono::high_resolution_clock;

class TickLoop
{
    bool isRunning = false;
    Clock::time_point start, previous;
    Clock::duration targetTimeStep;

public:
    Clock::duration RunDuration() const { return Clock::now() - start; }
    Clock::duration RunDelta() const { return Clock::now() - previous; }
    const Clock::duration& TargetTimeStep() const { return targetTimeStep; }
    bool IsRunning() const { return isRunning; }

    bool TickWait(Clock::duration& outDelta);
    void Reset(const Clock::duration& targetTimeStep = 16ms);
    void Stop();
};