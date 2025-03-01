#include "Shader.h"

Shader::Shader() : m_vertexShader(nullptr), m_pixelShader(nullptr), m_layout(nullptr), m_sampleState(nullptr), m_matrixBuffer(nullptr)
{
}

Shader::~Shader()
{
}

std::string Shader::GetShaderName()
{
	return std::string();
}

bool Shader::Init(ID3D11Device*, HWND)
{
	return false;
}

void Shader::Shutdown()
{
	ShutdownShader();
}

void Shader::ClearCache()
{
	m_shaderCache.clear();
}

bool Shader::InitShader(ID3D11Device*, HWND, std::wstring, std::wstring)
{
	return false;
}

void Shader::ShutdownShader()
{
	if (m_vertexShader)
	{
		m_vertexShader->Release();
		m_vertexShader = nullptr;
	}
	if (m_pixelShader)
	{
		m_pixelShader->Release();
		m_pixelShader = nullptr;
	}
	if (m_layout)
	{
		m_layout->Release();
		m_layout = nullptr;
	}
	if (m_sampleState)
	{
		m_sampleState->Release();
		m_sampleState = nullptr;
	}
	if (m_matrixBuffer)
	{
		m_matrixBuffer->Release();
		m_matrixBuffer = nullptr;
	}
	m_shaderCache.clear();
}

bool Shader::HandleShaderLoadingError(ID3D10Blob* errorMessage, HWND hwnd, std::wstring filename)
{
	if (errorMessage)
	{
		OutputShaderErrorMessage(errorMessage, hwnd, filename);
		return false;
	}
	MessageBox(hwnd, filename.c_str(), L"Missing Shader File", MB_OK);
	return false;
}

void Shader::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, std::wstring shaderFilename)
{
	char* compileErrors;
	compileErrors = (char*)(errorMessage->GetBufferPointer());

	unsigned long long bufferSize;
	bufferSize = errorMessage->GetBufferSize();

	ofstream fout;
	fout.open("shader-error.txt");

	for (int i = 0; i < bufferSize; i++)
	{
		fout << compileErrors[i];
	}

	fout.close();

	errorMessage->Release();
	errorMessage = 0;

	MessageBox(hwnd, L"Error compiling shader.  Check shader-error.txt for message.", shaderFilename.c_str(), MB_OK);
}

bool Shader::CreateSamplerDesc(ID3D11Device* device, D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressU, D3D11_TEXTURE_ADDRESS_MODE addressV,
	D3D11_TEXTURE_ADDRESS_MODE addressW, float mipLODBias, UINT maxAnisotropy, D3D11_COMPARISON_FUNC comparisonFunc, float borderColor0, float borderColor1,
	float borderColor2, float borderColor3, float minLOD, float maxLOD)
{
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = filter;
	samplerDesc.AddressU = addressU;
	samplerDesc.AddressV = addressV;
	samplerDesc.AddressW = addressW;
	samplerDesc.MipLODBias = mipLODBias;
	samplerDesc.MaxAnisotropy = maxAnisotropy;
	samplerDesc.ComparisonFunc = comparisonFunc;
	samplerDesc.BorderColor[0] = borderColor0;
	samplerDesc.BorderColor[1] = borderColor1;
	samplerDesc.BorderColor[2] = borderColor2;
	samplerDesc.BorderColor[3] = borderColor3;
	samplerDesc.MinLOD = minLOD;
	samplerDesc.MaxLOD = maxLOD;
	return !FAILED(device->CreateSamplerState(&samplerDesc, &m_sampleState));
}

void Shader::CheckSuccess(HRESULT hResult)
{
	if (FAILED(hResult))
	{
		throw "Failed to Init Shader" + GetShaderName();
	}
}
bool Shader::NeedsUpdating(std::string name, void* data)
{
	return (m_shaderCache.find(name) == m_shaderCache.end()) ||
		(m_shaderCache.find(name) != m_shaderCache.end() && m_shaderCache[name] != data);
}

void Shader::RenderShader(ID3D11DeviceContext*, int)
{
}
