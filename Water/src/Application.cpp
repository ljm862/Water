#include "Application.h"
#include <iostream>
Application::Application() : m_Direct3D(nullptr) {}
Application::~Application() {}

bool Application::Init(HWND hwnd)
{
	RECT r;
	if (!GetWindowRect(hwnd, &r)) return false;
	int width = r.right - r.left;
	int height = r.bottom - r.top;

	m_Direct3D = std::make_unique<D3DHandler>();
	if (!m_Direct3D->Init(width, height, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR))
	{
		MessageBox(hwnd, L"Could not initialize Direct3D", L"Error", MB_OK);
		return false;
	}

	m_Camera = std::make_unique<Camera>();
	m_Camera->SetPosition(0.0f, 2.0f, -15.0f);

	m_Model = std::make_unique<Model>();
	if (!m_Model->Init(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), "src/resources/models/plane.mdl")) return false;

	m_LightShader = std::make_unique<LightShader>();
	if (!m_LightShader->Init(m_Direct3D->GetDevice(), hwnd)) return false;
	m_ShaderList.push_back(m_LightShader.get());

	m_FontShader = std::make_unique<FontShader>();
	if (!m_FontShader->Init(m_Direct3D->GetDevice(), hwnd)) return false;
	m_ShaderList.push_back(m_FontShader.get());

	m_WorldLight = std::make_unique<Light>();
	m_WorldLight->SetAmbientColor(0.1f, 0.1f, 0.1f, 1.0f);
	m_WorldLight->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
	m_WorldLight->SetDirection(1.0f, 0.0f, 1.0f);
	m_WorldLight->SetSpecularColor(1.0f, 1.0f, 1.0f, 1.0f);
	m_WorldLight->SetSpecularPower(32.0f);

	m_FrameTimer = make_unique<FrameTimer>();
	if (!m_FrameTimer->Init()) return false;

	m_Font = make_unique<Font>();
	if (!m_Font->Init(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), 0)) return false;

	std::string fpsString = "FPS: 0";
	m_FpsString = make_unique<Text>();
	if (!m_FpsString->Init(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), width, height, 32, m_Font.get(), fpsString, 10, 10, XMFLOAT3(0.0f, 1.0f, 0.0f))) return false;

	std::string frameTimeString = "Frame: 0.0000ms";
	m_FrameTimeString = make_unique<Text>();
	if (!m_FrameTimeString->Init(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), width, height, 32, m_Font.get(), frameTimeString, 10, 10, XMFLOAT3(0.0f, 1.0f, 0.0f))) return false;

	return true;
}

void Application::Shutdown()
{

	if (m_Model)
	{
		m_Model->Shutdown();
	}

	if (m_Direct3D)
	{
		m_Direct3D->Shutdown();
	}

	if (m_LightShader)
	{
		m_LightShader->Shutdown();
	}

	if (m_FontShader)
	{
		m_FontShader->Shutdown();
	}

	if (m_Font)
	{
		m_Font->Shutdown();
	}

	if (m_FpsString)
	{
		m_FpsString->Shutdown();
	}

	if (m_FrameTimeString)
	{
		m_FrameTimeString->Shutdown();
	}


	m_WorldLight.reset();
	m_LightShader.reset();
	m_FontShader.reset();
	m_Camera.reset();
	m_Direct3D.reset();
	m_Model.reset();
	m_FrameTimeString.reset();
	m_FpsString.reset();
	m_FrameTimer.reset();
	m_Font.reset();
}

bool Application::Run()
{
	ClearShaderCaches();
	UpdateFps();
	return ProcessFrame();
}

// Should change this to a Renderer class that places all the models first, then does the directional lights,
// then does the point lights and then any post processing.
// Then a 2d renderer on top of that which does all the UI stuff after
bool Application::ProcessFrame() {
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix, translateMatrix;

	m_Direct3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	m_Camera->Render();

	m_Direct3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	m_Direct3D->GetProjectionMatrix(projectionMatrix);
	XMMATRIX orthoMatrix, worldMatrix2d, viewMatrix2d;
	m_Direct3D->GetWorldMatrix(worldMatrix2d);
	m_Camera->GetViewMatrix(viewMatrix2d);
	m_Direct3D->GetOrthoMatrix(orthoMatrix);

	translateMatrix = XMMatrixTranslation(0.0f, 0.0f, 0.0f);
	worldMatrix = XMMatrixMultiply(worldMatrix, translateMatrix);

	if (!RenderObject(worldMatrix, viewMatrix, projectionMatrix, m_Model.get())) return false;

	/************* 2D Render *****************/
	m_Direct3D->TurnZBufferOff();

	// If the viewMatrix is not a simple default one, rectify that here as it needs to point to the camera.

	///////// UI /////////////

	m_Direct3D->EnableAlphaBlending();
	m_FpsString->Render(m_Direct3D->GetDeviceContext());
	if (!m_FontShader->Render(m_Direct3D->GetDeviceContext(), m_FpsString->GetIndexCount(), worldMatrix2d, viewMatrix2d, orthoMatrix, m_Font->GetTexture(), m_FpsString->GetPixelColor())) return false;

	m_FrameTimeString->Render(m_Direct3D->GetDeviceContext());
	if (!m_FontShader->Render(m_Direct3D->GetDeviceContext(), m_FrameTimeString->GetIndexCount(), worldMatrix2d, viewMatrix2d, orthoMatrix, m_Font->GetTexture(), m_FrameTimeString->GetPixelColor())) return false;

	m_Direct3D->DisableAlphaBlending();
	/////// END UI //////////

	m_Direct3D->TurnZBufferOn();

	/*********** 2D Render Off ***************/

	m_Direct3D->EndScene();
	return true;
}

bool Application::RenderObject(XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, Model* model)
{
	model->Render(m_Direct3D->GetDeviceContext());
	return m_LightShader->Render(m_Direct3D->GetDeviceContext(), model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, m_WorldLight->GetDirection(), m_WorldLight->GetDiffuseColor(), m_WorldLight->GetAmbientColor(),
		m_Camera->GetPosition(), m_WorldLight->GetSpecularColor(), m_WorldLight->GetSpecularPower());
}

bool Application::UpdateFps()
{
	auto previousFps = m_FrameTimer->GetFps();
	m_FrameTimer->Frame();
	auto frameTime = m_FrameTimer->GetTime();
	auto currentFps = m_FrameTimer->GetFps();

	if (previousFps == currentFps) return true;
	if (currentFps > 99999) currentFps = 99999;

	char tempString[16];
	sprintf_s(tempString, "%d", currentFps);

	char finalString[16];
	strcpy_s(finalString, "Fps: ");
	strcat_s(finalString, tempString);

	char frameString[32];
	sprintf_s(tempString, "%f", frameTime * 1000);
	strcpy_s(frameString, "Frame: ");
	strcat_s(frameString, tempString);
	strcat_s(frameString, "ms");

	auto color = GetFpsColor(currentFps);

	m_FpsString->SetText(finalString);
	if (!m_FpsString->UpdateText(m_Direct3D->GetDeviceContext(), m_Font.get(), 100, 10, color)) return false;

	m_FrameTimeString->SetText(frameString);
	if (!m_FrameTimeString->UpdateText(m_Direct3D->GetDeviceContext(), m_Font.get(), 100, 50, color)) return false;

	return true;
}

XMFLOAT3 Application::GetFpsColor(int fps)
{
	if (fps >= 120)
	{
		return XMFLOAT3(0.0f, 1.0f, 0.0f);
	}

	if (fps < 120)
	{
		return XMFLOAT3(1.0f, 1.0f, 0.0f);
	}

	if (fps < 60)
	{
		return XMFLOAT3(1.0f, 0.0f, 0.0f);
	}
	return XMFLOAT3(1.0f, 1.0f, 1.0f);
}

void Application::ClearShaderCaches()
{
	for (int i = 0; i < m_ShaderList.size(); i++)
	{
		m_ShaderList[i]->ClearCache();
	}
}
