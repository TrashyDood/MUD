#pragma once

#include <chrono>

using namespace std::chrono_literals;
using TickLoopClock = std::chrono::high_resolution_clock;
using TickLoopDuration = TickLoopClock::duration;

constexpr TickLoopDuration DefaultTimeStep = 20ms;

class TickLoop
{
    bool isRunning = false;
    TickLoopClock::time_point start, previous;
    TickLoopDuration targetTimeStep;

public:
    TickLoopDuration CurrentDuration() const { return TickLoopClock::now() - start; }
    TickLoopDuration CurrentDelta() const { return TickLoopClock::now() - previous; }
    bool IsRunning() const { return isRunning; }
    const TickLoopDuration &GetTargetTimeStep() const { return targetTimeStep; }

    bool WaitForTick(TickLoopDuration &outDelta, TickLoopDuration &outDuration, TickLoopDuration targetTimeStep = DefaultTimeStep);
    void Start(const TickLoopDuration &targetTimeStep = DefaultTimeStep);
    void Stop();
};