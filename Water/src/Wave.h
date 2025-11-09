#pragma once
#include <chrono>
#include "FrameTimer.h"
#include <directxmath.h>

const float DEG2RAD = 0.0174533;
using namespace DirectX;

class Wave
{
public:
    struct OptimisedWaveType
    {
        XMFLOAT2 direction;
        XMFLOAT2 origin;
        XMFLOAT4 fapsProperties; // x=frequency, y=amplitude, z=phase, w=steepness 
        XMFLOAT4 time; // x=time, others are padding
    };

public:
    Wave();
    ~Wave();

    bool Init(float wavelength, float amplitude, float speed, float direction, XMFLOAT2 position);
    void UpdateTime();

    float GetTime();

    OptimisedWaveType GetOptimisedBuffer();

private:
    XMFLOAT2 m_direction;
    XMFLOAT2 m_position;
    float m_frequency;
    float m_amplitude;
    float m_phase;
    float m_steepness;
    float m_time;
    std::chrono::time_point<std::chrono::steady_clock> m_startTime;
};

