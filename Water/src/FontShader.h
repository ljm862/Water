#pragma once
#include "Shader.h"
#include "Utils.h"
class FontShader : public Shader
{
private:
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
	};

	struct PixelBufferType
	{
		XMFLOAT4 pixelColor;
	};

public:
	FontShader();
	~FontShader();

	std::string GetShaderName() override;
	bool Init(ID3D11Device*, HWND) override;
	bool Render(ID3D11DeviceContext*, int, XMMATRIX, XMMATRIX, XMMATRIX, ID3D11ShaderResourceView*, XMFLOAT4);

private:
	bool InitShader(ID3D11Device*, HWND, std::wstring, std::wstring) override;
	void ShutdownShader() override;
	void RenderShader(ID3D11DeviceContext*, int) override;
	bool SetShaderParameters(ID3D11DeviceContext*, XMMATRIX, XMMATRIX, XMMATRIX, ID3D11ShaderResourceView*, XMFLOAT4);

private:
	ID3D11Buffer* m_pixelBuffer;
};

