#include "TickLoop.h"

bool TickLoop::WaitForTick(Duration targetTimeStep)
{
    if (!isRunning)
        Start(targetTimeStep);

    Duration accumulator = 0s;

    while (accumulator < targetTimeStep)
    {
        auto now = Clock::now();
        accumulator += now - previous;
        previous = now;
    }

    timeStep = accumulator;

    return isRunning;
}

void TickLoop::Start(const Duration &targetTimeStep)
{
    TickLoop::targetTimeStep = targetTimeStep;
    start = Clock::now();
    isRunning = true;
}

void TickLoop::Stop()
{
    isRunning = false;
}
