#include "TickLoop.h"

bool TickLoop::TickWait(Clock::duration& outDelta)
{
    outDelta = 0s;

    while (outDelta < targetTimeStep)
    {
        auto now = Clock::now();
        outDelta += RunDelta();
        previous = now;
    }

    return isRunning;
}

void TickLoop::Reset(const Clock::duration& targetTimeStep)
{
    TickLoop::targetTimeStep = targetTimeStep;
    start = Clock::now();
    isRunning = true;
}

void TickLoop::Stop()
{
    isRunning = false;
}
