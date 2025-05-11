#include "Time.h"

namespace fw
{

void Time::update()
{
    sys::moment now = sys::now();
    sm_deltaTime = std::chrono::duration_cast<std::chrono::nanoseconds>(now - sm_lastFrameBegin).count() / 1e9;
    sm_lastFrameBegin = now;
}

f64 Time::delta_time()
{
    return sm_deltaTime;
}

sys::moment Time::sm_lastFrameBegin = sys::now();
f64 Time::sm_deltaTime = 0.0;

} // fw