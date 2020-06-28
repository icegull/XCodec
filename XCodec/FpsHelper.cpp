#include "FpsHelper.h"
#include <windows.h>

void FpsHelper::getCPUfreq(uint64_t& _freq)
{
	QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&_freq));
}

void FpsHelper::getTickCount(uint64_t& _tick)
{
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&_tick));
}

FpsHelper::FpsHelper()
{
	getCPUfreq(m_freq);
	getTickCount(m_tick);
}

int32_t FpsHelper::update_fps()
{
	uint64_t tick;
	getTickCount(tick);
	if (tick - m_tick > m_freq)
	{
		m_tick = tick;
		m_fps = m_cpV;
		m_cpV = 0;
	}
	++m_cpV;
	return m_fps;
}

int32_t FpsHelper::getFps() const
{
	return m_fps;
}