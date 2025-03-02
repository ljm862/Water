#include "LightShader.h"

LightShader::LightShader() : m_lightBuffer(nullptr), m_cameraBuffer(nullptr), m_lightColorBuffer(nullptr), m_lightPositionBuffer(nullptr)
{

}

LightShader::~LightShader()
{
}

std::string LightShader::GetShaderName()
{
	return "LightShader";
}

bool LightShader::Init(ID3D11Device* device, HWND hwnd)
{
	std::wstring vsf = L"src/shaders/light.vs";
	std::vector<wchar_t> vertexShaderFilename(vsf.begin(), vsf.end());
	vertexShaderFilename.push_back(L'\0');

	std::wstring psf = L"src/shaders/light.ps";
	std::vector<wchar_t> pixelShaderFilename(psf.begin(), psf.end());
	pixelShaderFilename.push_back(L'\0');

	return InitShader(device, hwnd, vertexShaderFilename.data(), pixelShaderFilename.data());
}

bool LightShader::Render(ID3D11DeviceContext* deviceContext, int indexCount, XMMATRIX wMatrix, XMMATRIX vMatrix, XMMATRIX pMatrix, ID3D11ShaderResourceView* texture,
	XMFLOAT3 lightDirection, XMFLOAT4 diffuseColor, XMFLOAT4 ambientColor, XMFLOAT3 cameraPosition,
	XMFLOAT4 specularColor, float specularPower)
{
	if (!SetShaderParameters(deviceContext, wMatrix, vMatrix, pMatrix, texture, lightDirection, diffuseColor, ambientColor, cameraPosition, specularColor, specularPower)) return false;

	RenderShader(deviceContext, indexCount);
	return true;
}

bool LightShader::InitShader(ID3D11Device* device, HWND hwnd, std::wstring vsFilename, std::wstring psFilename)
{
	HRESULT result;
	ID3D10Blob* errorMessage(0);
	ID3D10Blob* vertexShaderBuffer(0);
	ID3D10Blob* pixelShaderBuffer(0);
	result = D3DCompileFromFile(vsFilename.c_str(), nullptr, nullptr, "LightVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &vertexShaderBuffer, &errorMessage);
	if (FAILED(result))return Shader::HandleShaderLoadingError(errorMessage, hwnd, vsFilename);

	result = D3DCompileFromFile(psFilename.c_str(), nullptr, nullptr, "LightPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pixelShaderBuffer, &errorMessage);
	if (FAILED(result))return Shader::HandleShaderLoadingError(errorMessage, hwnd, psFilename);

	CheckSuccess(device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), nullptr, &m_vertexShader));
	CheckSuccess(device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), nullptr, &m_pixelShader));

	std::vector<D3D11_INPUT_ELEMENT_DESC> polygonLayout(3);
	Utils::GenerateElementDesc(&polygonLayout, 0, "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0);
	Utils::GenerateElementDesc(&polygonLayout, 1, "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0);
	Utils::GenerateElementDesc(&polygonLayout, 2, "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0);

	unsigned int numElements = polygonLayout.size();

	CheckSuccess(device->CreateInputLayout(polygonLayout.data(), numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_layout));

	vertexShaderBuffer->Release();
	vertexShaderBuffer = nullptr;
	pixelShaderBuffer->Release();
	pixelShaderBuffer = nullptr;

	CheckSuccess(Shader::CreateSamplerDesc(device, D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP, 0.0f, 1,
		D3D11_COMPARISON_ALWAYS, 0, 0, 0, 0, 0, D3D11_FLOAT32_MAX));

	if (Utils::SetupAndCreateBuffer(device, &m_matrixBuffer, D3D11_USAGE_DYNAMIC, sizeof(MatrixBufferType), D3D11_BIND_CONSTANT_BUFFER, D3D11_CPU_ACCESS_WRITE, 0, 0)) return false;
	if (Utils::SetupAndCreateBuffer(device, &m_lightBuffer, D3D11_USAGE_DYNAMIC, sizeof(LightBufferType), D3D11_BIND_CONSTANT_BUFFER, D3D11_CPU_ACCESS_WRITE, 0, 0)) return false;
	if (Utils::SetupAndCreateBuffer(device, &m_cameraBuffer, D3D11_USAGE_DYNAMIC, sizeof(CameraBufferType), D3D11_BIND_CONSTANT_BUFFER, D3D11_CPU_ACCESS_WRITE, 0, 0)) return false;

	return true;
}

void LightShader::ShutdownShader()
{
	Shader::ShutdownShader();
	if (m_lightBuffer)
	{
		m_lightBuffer->Release();
		m_lightBuffer = 0;
	}
	if (m_cameraBuffer)
	{
		m_cameraBuffer->Release();
		m_cameraBuffer = 0;
	}
	if (m_lightColorBuffer)
	{
		m_lightColorBuffer->Release();
		m_lightColorBuffer = 0;
	}
	if (m_lightPositionBuffer)
	{
		m_lightPositionBuffer->Release();
		m_lightPositionBuffer = 0;
	}
}

bool LightShader::SetShaderParameters(ID3D11DeviceContext* deviceContext, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, ID3D11ShaderResourceView* texture,
	XMFLOAT3 lightDirection, XMFLOAT4 diffuseColor, XMFLOAT4 ambientColor, XMFLOAT3 cameraPosition, XMFLOAT4 specularColor, float specularPower)
{

	worldMatrix = XMMatrixTranspose(worldMatrix);
	viewMatrix = XMMatrixTranspose(viewMatrix);
	projectionMatrix = XMMatrixTranspose(projectionMatrix);

	/************************ VERTEX SHADERS *************************/

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	unsigned int bufferNumber = 0;

	MatrixBufferType tmpMatrix;
	tmpMatrix.world = worldMatrix;
	tmpMatrix.view = viewMatrix;
	tmpMatrix.projection = projectionMatrix;
	if (NeedsUpdating("model", &tmpMatrix, sizeof(MatrixBufferType)))
	{
		CheckSuccess(deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		auto matrixBufferPtr = (MatrixBufferType*)mappedResource.pData;
		matrixBufferPtr->world = worldMatrix;
		matrixBufferPtr->view = viewMatrix;
		matrixBufferPtr->projection = projectionMatrix;

		deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);
		deviceContext->Unmap(m_matrixBuffer, 0);

		m_shaderCache["model"] = &tmpMatrix;
	}
	bufferNumber = 1;

	CameraBufferType tmpCamera;
	tmpCamera.cameraPoisition = cameraPosition;
	tmpCamera.padding = 0.0f;
	if (NeedsUpdating("camera", &tmpCamera, sizeof(CameraBufferType)))
	{
		CheckSuccess(deviceContext->Map(m_cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		auto cameraBufferPtr = (CameraBufferType*)mappedResource.pData;
		cameraBufferPtr->cameraPoisition = cameraPosition;
		cameraBufferPtr->padding = 0.0f;// padds the buffer to be divisble by 16

		deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_cameraBuffer);
		deviceContext->Unmap(m_cameraBuffer, 0);

		m_shaderCache["camera"] = &tmpCamera;
	}

	/************************* PIXEL SHADERS ************************/

	bufferNumber = 0;
	deviceContext->PSSetShaderResources(bufferNumber, 1, &texture);

	bufferNumber = 0;

	LightBufferType tmpLight;
	tmpLight.ambientColor = ambientColor;
	tmpLight.diffuseColor = diffuseColor;
	tmpLight.lightDirection = lightDirection;
	tmpLight.specularColor = specularColor;
	tmpLight.specularPower = specularPower;
	if (NeedsUpdating("light", &tmpLight, sizeof(LightBufferType)))
	{
		CheckSuccess(deviceContext->Map(m_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		auto lightBufferPtr = (LightBufferType*)mappedResource.pData;
		lightBufferPtr->ambientColor = ambientColor;
		lightBufferPtr->diffuseColor = diffuseColor;
		lightBufferPtr->lightDirection = lightDirection;
		lightBufferPtr->specularColor = specularColor;
		lightBufferPtr->specularPower = specularPower;

		deviceContext->PSSetConstantBuffers(bufferNumber, 1, &m_lightBuffer);
		deviceContext->Unmap(m_lightBuffer, 0);

		m_shaderCache["light"] = &tmpLight;
	}

	return true;
}

void LightShader::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
	deviceContext->IASetInputLayout(m_layout);

	deviceContext->VSSetShader(m_vertexShader, nullptr, 0);
	deviceContext->PSSetShader(m_pixelShader, nullptr, 0);

	deviceContext->PSSetSamplers(0, 1, &m_sampleState);
	deviceContext->DrawIndexed(indexCount, 0, 0);
}
