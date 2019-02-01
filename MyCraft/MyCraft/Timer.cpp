#include "Timer.h"

Timer::Timer() {
    QueryPerformanceFrequency(&m_counterFreq);
    QueryPerformanceCounter(&m_startTime);
}

double Timer::Tick() {
    LARGE_INTEGER liCurrent;
    QueryPerformanceCounter(&liCurrent);
    
    m_elapseTime = (liCurrent.QuadPart - m_startTime.QuadPart) / static_cast<double>(m_counterFreq.QuadPart);
    
    // Time between  ticks.
    double t = (liCurrent.QuadPart - m_lastTime.QuadPart) / static_cast<double>(m_counterFreq.QuadPart);

    m_lastTime = liCurrent;

    return t;
}