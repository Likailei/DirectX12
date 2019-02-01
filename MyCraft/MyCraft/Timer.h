#pragma once

#ifndef __TIMER_H__
#define __TIMER_H__

#include <Windows.h>

class Timer {
public:
    Timer();

    double Tick();
    double GetElapseTime() { return m_elapseTime; }
private:
    LARGE_INTEGER m_counterFreq;
    LARGE_INTEGER m_startTime;
    double m_elapseTime;
    LARGE_INTEGER m_lastTime;
};
#endif // !__TIMER_H__
