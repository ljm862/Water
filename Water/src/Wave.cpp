#include "Wave.h"

Wave::Wave()
{
}

bool Wave::Init(float wavelength, float amplitude, float speed, float direction, XMFLOAT2 position)
{
	m_frequency = 2 / wavelength;
	m_amplitude = amplitude;
	m_phase = speed * m_frequency;
	m_position = position;
	m_startTime = std::chrono::steady_clock::now();
	m_direction = XMFLOAT2(std::cos(DEG2RAD * direction), std::sin(DEG2RAD * direction));
	return true;
}

Wave::~Wave()
{
}

void Wave::UpdateTime()
{
	auto currentTime = std::chrono::high_resolution_clock::now();
	m_time = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - m_startTime).count() / 1000.0f;
	//TODO: Check this time resolution
}

Wave::OptimisedWaveType Wave::GetOptimisedBuffer()
{
	OptimisedWaveType owt;
	owt.direction = m_direction;
	owt.origin = m_position;
	owt.fapsProperties = XMFLOAT4(m_frequency, m_amplitude, m_phase, m_steepness);
	owt.time = XMFLOAT4(m_time, 0, 0, 0);
	return owt;
}
