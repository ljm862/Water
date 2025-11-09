#pragma once
#include <algorithm>

#include "Shader.h"
#include "Utils.h"
#include "Wave.h"

const int NUM_WAVES = 64;
class WaterShader : public Shader
{
private:
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
	};

	struct WaveBufferType
	{
		Wave::OptimisedWaveType waves[NUM_WAVES];
	};


	struct CameraBufferType
	{
		XMFLOAT3 cameraPoisition;
		float padding;
	};

	struct LightBufferType
	{
		XMFLOAT4 ambientColor;
		XMFLOAT4 diffuseColor;
		XMFLOAT3 lightDirection;
		float specularPower; // Has to be here to work in groups of 16
		XMFLOAT4 specularColor;
	};

public:
	WaterShader();
	~WaterShader();
	std::string GetShaderName() override;
	bool Init(ID3D11Device*, HWND) override;
	bool Render(ID3D11DeviceContext*, int, XMMATRIX, XMMATRIX, XMMATRIX, XMFLOAT3, XMFLOAT3, XMFLOAT4, XMFLOAT4, XMFLOAT4, float, std::vector<Wave>);

private:
	bool InitShader(ID3D11Device*, HWND, std::wstring, std::wstring) override;
	void ShutdownShader() override;
	void RenderShader(ID3D11DeviceContext*, int) override;
	bool SetShaderParameters(ID3D11DeviceContext*, XMMATRIX, XMMATRIX, XMMATRIX, LightBufferType, WaveBufferType, XMFLOAT3);


private:
	ID3D11Buffer* m_lightBuffer;
	ID3D11Buffer* m_waveBuffer;
	ID3D11Buffer* m_cameraBuffer;

};

