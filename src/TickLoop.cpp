#include "TickLoop.h"

bool TickLoop::WaitForTick(TickLoopDuration &outDelta, TickLoopDuration &outDuration, TickLoopDuration targetTimeStep)
{
    if (!isRunning)
        Start(targetTimeStep);

    outDelta = 0s;

    while (outDelta < targetTimeStep)
    {
        auto now = TickLoopClock::now();
        outDelta += CurrentDelta();
        previous = now;
    }

    outDuration = CurrentDuration();

    return isRunning;
}

void TickLoop::Start(const TickLoopDuration &targetTimeStep)
{
    TickLoop::targetTimeStep = targetTimeStep;
    start = TickLoopClock::now();
    isRunning = true;
}

void TickLoop::Stop()
{
    isRunning = false;
}
