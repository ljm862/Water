#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <fstream>
#include <vector>
#include <map>

using namespace DirectX;
using namespace std;
class Shader
{
public:
	Shader();
	virtual ~Shader();

	virtual std::string GetShaderName();
	virtual bool Init(ID3D11Device*, HWND);
	virtual void Shutdown();
	void ClearCache();


protected:

	bool HandleShaderLoadingError(ID3D10Blob*, HWND, std::wstring);
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, std::wstring);
	bool CreateSamplerDesc(ID3D11Device*, D3D11_FILTER, D3D11_TEXTURE_ADDRESS_MODE, D3D11_TEXTURE_ADDRESS_MODE, D3D11_TEXTURE_ADDRESS_MODE, float, UINT, D3D11_COMPARISON_FUNC, float, float, float, float, float, float);
	void CheckSuccess(HRESULT);
	bool NeedsUpdating(std::string, void*, size_t);
	virtual void RenderShader(ID3D11DeviceContext*, int);
	virtual void ShutdownShader();
	virtual bool InitShader(ID3D11Device*, HWND, std::wstring, std::wstring);




public:
	std::map<std::string, void*> m_shaderCache;

	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11SamplerState* m_sampleState;
	ID3D11Buffer* m_matrixBuffer;

};