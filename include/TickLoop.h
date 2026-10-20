#pragma once

#include <chrono>

using namespace std::chrono_literals;

class TickLoop
{
public:
    using Clock = std::chrono::high_resolution_clock;
    using Duration = Clock::duration;

private:
    bool isRunning = false;
    Clock::time_point start, previous;
    Duration targetTimeStep, timeStep;

public:
    bool IsRunning() const { return isRunning; }
    Duration ElapsedTime() const { return Clock::now() - start; }
    const Duration &TimeStep() const { return timeStep; }
    const Duration &TargetTimeStep() const { return targetTimeStep; }

    bool WaitForTick(Duration targetTimeStep = 20ms);
    void Start(const Duration &targetTimeStep = 20ms);
    void Stop();
};