#include "FrameTimer.h"

FrameTimer::FrameTimer() : m_fps(0), m_count(0)
{
}

FrameTimer::~FrameTimer()
{
}

bool FrameTimer::Init()
{
	INT64 frequency;
	QueryPerformanceFrequency((LARGE_INTEGER*)&frequency);
	if (frequency == 0)return false;

	m_frequency = (float)frequency;

	QueryPerformanceCounter((LARGE_INTEGER*)&m_windowStartTime);
	m_fpsStartTime = timeGetTime();
	return true;
}

void FrameTimer::Frame()
{
	m_count++;

	INT64 currentTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);

	INT64 elapsedTicks;
	elapsedTicks = currentTime - m_windowStartTime;

	m_frameTime = (float)elapsedTicks / m_frequency;
	m_windowStartTime = currentTime;

	auto now = timeGetTime();
	if (now >= (m_fpsStartTime + 1000))
	{
		m_fps = m_count;
		m_count = 0;
		m_fpsStartTime = now;
	}
}

float FrameTimer::GetTime()
{
	return m_frameTime;
}

int FrameTimer::GetFps()
{
	return m_fps;
}