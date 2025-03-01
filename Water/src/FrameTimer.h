#pragma once
#pragma comment(lib, "winmm.lib")

#include <Windows.h>
#include <mmsystem.h>
class FrameTimer
{
public:
	FrameTimer();
	~FrameTimer();

	bool Init();
	void Frame();

	float GetTime();
	int GetFps();

private:
	float m_frequency;
	INT64 m_windowStartTime;
	float m_frameTime;

	int m_fps, m_count;
	unsigned long m_fpsStartTime;
};

