#pragma once
#include "system/timer.h"

namespace fw
{

class Time
{
public:
    static void update();

    static f64 delta_time();
private:
    static sys::moment sm_lastFrameBegin;
    static f64 sm_deltaTime;
private:
    Time() = delete;
    ~Time() = delete;
};

} // fw