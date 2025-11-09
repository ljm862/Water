#include "Wave.h"

#define DEBUG
#ifdef DEBUG
#include <iostream>
#endif

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

#ifdef DEBUG
	std::cout << "Wave Info:" << std::endl;
	std::cout << "Freq: " << m_frequency << ". Amp: " << m_amplitude << ". Phase: " << m_phase << ". Position: " << m_position.x << ","
		<< m_position.y << "," << ". Direction: " << m_direction.x << "," << m_direction.y << ". Wavelength: " << 2/m_frequency << std::endl;
#endif
	return true;
}

Wave::~Wave()
{
}

float Wave::GetTime()
{
	return m_time;
}

void Wave::UpdateTime()
{
	auto currentTime = std::chrono::high_resolution_clock::now();
	m_time = std::chrono::duration<float>(currentTime - m_startTime).count();
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
