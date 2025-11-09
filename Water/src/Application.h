#pragma once

#include <memory>
#include <Windows.h>
#include <random>

#include "D3DHandler.h"
#include "Camera.h"
#include "Model.h"
#include "Light.h"
#include "LightShader.h"
#include "Wave.h"
#include "WaterShader.h"

#include "FrameTimer.h"
#include "UI/Font.h"
#include "UI/FontShader.h"
#include "UI/Text.h"

#define DEBUG

const bool FULL_SCREEN = false;
const int VSYNC_ENABLED = 0;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.3f;
class Application
{
public:
	Application();
	~Application();

	bool Init(HWND hwnd);
	void Shutdown();
	bool Run();

private:
	bool ProcessFrame();
	bool RenderObject(XMMATRIX, XMMATRIX, XMMATRIX, Model*);
	bool UpdateFps();
	XMFLOAT3 GetFpsColor(int);
	void GenerateWaves();
	void UpdateWaveTimes();

private:
	std::unique_ptr<D3DHandler> m_Direct3D;

	std::unique_ptr<Camera> m_Camera;
	std::unique_ptr<Model> m_Model;
	std::unique_ptr<Light> m_WorldLight;

	std::unique_ptr<LightShader> m_LightShader;
	std::unique_ptr<FontShader> m_FontShader;
	std::unique_ptr<WaterShader> m_WaterShader;
	std::vector<Shader*> m_ShaderList;

	std::vector<Wave> m_Waves;

	std::unique_ptr<FrameTimer> m_FrameTimer;
	std::unique_ptr<Font> m_Font;
	std::unique_ptr<Text> m_FpsString;
	std::unique_ptr<Text> m_FrameTimeString;
};

