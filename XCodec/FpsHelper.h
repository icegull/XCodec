#pragma once
#include <cstdint>

class FpsHelper
{
	uint64_t m_freq = 0;
	uint64_t m_tick = 0;
	int32_t  m_cpV = 0;
	int32_t	 m_fps = 0;

	static void getCPUfreq(uint64_t& _freq);
	static void getTickCount(uint64_t& _tick);
public:
	FpsHelper();
	~FpsHelper() = default;
	int32_t	update_fps();
	int32_t getFps() const;
};
