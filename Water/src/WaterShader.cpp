#include "WaterShader.h"

WaterShader::WaterShader()
{
}

WaterShader::~WaterShader()
{
}

std::string WaterShader::GetShaderName()
{
	return "WaterShader";
}

bool WaterShader::Init(ID3D11Device* device, HWND hwnd)
{
	std::wstring vsf = L"src/shaders/Water.vs";
	std::vector<wchar_t> vertexShaderFilename(vsf.begin(), vsf.end());
	vertexShaderFilename.push_back(L'\0');

	std::wstring psf = L"src/shaders/Water.ps";
	std::vector<wchar_t> pixelShaderFilename(psf.begin(), psf.end());
	pixelShaderFilename.push_back(L'\0');

	return InitShader(device, hwnd, vertexShaderFilename.data(), pixelShaderFilename.data());
}

bool WaterShader::Render(ID3D11DeviceContext* deviceContext, int indexCount, XMMATRIX wMatrix, XMMATRIX vMatrix, XMMATRIX pMatrix, XMFLOAT3 lightDirection, XMFLOAT4 diffuseColor, XMFLOAT4 ambientColor,
	XMFLOAT4 specularColor, float specularPower, std::vector<Wave> waves)
{
	LightBufferType lightBufferObj;
	lightBufferObj.lightDirection = lightDirection;
	lightBufferObj.diffuseColor = diffuseColor;
	lightBufferObj.ambientColor = ambientColor;
	lightBufferObj.specularColor = specularColor;
	lightBufferObj.specularPower = specularPower;

	WaveBufferType waveBufferObj;
	for (int i = 0; i < waves.size(); i++)
	{
		waveBufferObj.waves[i] = waves[i].GetOptimisedBuffer();
	}

	if (!SetShaderParameters(deviceContext, wMatrix, vMatrix, pMatrix, lightBufferObj, waveBufferObj)) return false;

	RenderShader(deviceContext, indexCount);
	return true;
}

bool WaterShader::InitShader(ID3D11Device* device, HWND hwnd, std::wstring vsFilename, std::wstring psFilename)
{
	ID3D10Blob* errorMessage(0);
	ID3D10Blob* vertexShaderBuffer(0);
	ID3D10Blob* pixelShaderBuffer(0);
	if (FAILED(D3DCompileFromFile(vsFilename.c_str(), nullptr, nullptr, "WaterVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &vertexShaderBuffer, &errorMessage)))
		return Shader::HandleShaderLoadingError(errorMessage, hwnd, vsFilename);

	if (FAILED(D3DCompileFromFile(psFilename.c_str(), nullptr, nullptr, "WaterPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pixelShaderBuffer, &errorMessage)))
		return Shader::HandleShaderLoadingError(errorMessage, hwnd, psFilename);

	CheckSuccess(device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), nullptr, &m_vertexShader));
	CheckSuccess(device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), nullptr, &m_pixelShader));

	/*    float4 position : POSITION;
	float3 normal : NORMAL;
	float time : TIME;*/
	std::vector<D3D11_INPUT_ELEMENT_DESC> polygonLayout(3);
	Utils::GenerateElementDesc(&polygonLayout, 0, "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0);
	Utils::GenerateElementDesc(&polygonLayout, 1, "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0);
	Utils::GenerateElementDesc(&polygonLayout, 2, "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0);

	CheckSuccess(device->CreateInputLayout(polygonLayout.data(), polygonLayout.size(), vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_layout));

	vertexShaderBuffer->Release();
	vertexShaderBuffer = nullptr;
	pixelShaderBuffer->Release();
	pixelShaderBuffer = nullptr;

	if (Utils::SetupAndCreateBuffer(device, &m_matrixBuffer, D3D11_USAGE_DYNAMIC, sizeof(MatrixBufferType), D3D11_BIND_CONSTANT_BUFFER, D3D11_CPU_ACCESS_WRITE, 0, 0)) return false;
	if (Utils::SetupAndCreateBuffer(device, &m_waveBuffer, D3D11_USAGE_DYNAMIC, sizeof(MatrixBufferType), D3D11_BIND_CONSTANT_BUFFER, D3D11_CPU_ACCESS_WRITE, 0, 0)) return false;
	if (Utils::SetupAndCreateBuffer(device, &m_lightBuffer, D3D11_USAGE_DYNAMIC, sizeof(LightBufferType), D3D11_BIND_CONSTANT_BUFFER, D3D11_CPU_ACCESS_WRITE, 0, 0)) return false;

	return true;
}

void WaterShader::ShutdownShader()
{
	Shader::ShutdownShader();
	if (m_lightBuffer)
	{
		m_lightBuffer->Release();
		m_lightBuffer = nullptr;
	}
	if (m_matrixBuffer)
	{
		m_matrixBuffer->Release();
		m_matrixBuffer = nullptr;
	}
	if (m_waveBuffer)
	{
		m_waveBuffer->Release();
		m_waveBuffer = nullptr;
	}
}

void WaterShader::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
	deviceContext->IASetInputLayout(m_layout);

	deviceContext->VSSetShader(m_vertexShader, nullptr, 0);
	deviceContext->PSSetShader(m_pixelShader, nullptr, 0);

	deviceContext->DrawIndexed(indexCount, 0, 0);
}

bool WaterShader::SetShaderParameters(ID3D11DeviceContext* deviceContext, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, LightBufferType lightBufferObj, WaveBufferType waveBufferObj)
{
	worldMatrix = XMMatrixTranspose(worldMatrix);
	viewMatrix = XMMatrixTranspose(viewMatrix);
	projectionMatrix = XMMatrixTranspose(projectionMatrix);
	/************************ VERTEX SHADER *************************/

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	unsigned int bufferNumber = 0;

	MatrixBufferType tmpMatrix;
	tmpMatrix.world = worldMatrix;
	tmpMatrix.view = viewMatrix;
	tmpMatrix.projection = projectionMatrix;
	//if (NeedsUpdating("model", &tmpMatrix, sizeof(MatrixBufferType)))
	//{
		CheckSuccess(deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		auto matrixBufferPtr = (MatrixBufferType*)mappedResource.pData;
		matrixBufferPtr->world = worldMatrix;
		matrixBufferPtr->view = viewMatrix;
		matrixBufferPtr->projection = projectionMatrix;

		deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);
		deviceContext->Unmap(m_matrixBuffer, 0);

		m_shaderCache["model"] = &tmpMatrix;
	//}

	bufferNumber = 1;

	//WaveBufferType tmpWave;
	//std::copy(std::begin(waveBufferObj.waves), std::end(waveBufferObj.waves), std::begin(tmpWave.waves));
	//if (NeedsUpdating("waves", &waveBufferObj, sizeof(WaveBufferType)))
	//{
		CheckSuccess(deviceContext->Map(m_waveBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		auto waveBufferPtr = (WaveBufferType*)mappedResource.pData;
		std::copy(std::begin(waveBufferObj.waves), std::end(waveBufferObj.waves), std::begin(waveBufferPtr->waves));

		deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_waveBuffer);
		deviceContext->Unmap(m_waveBuffer, 0);

		m_shaderCache["waves"] = &waveBufferObj;
	//}

	/************************* PIXEL SHADER ************************/

	bufferNumber = 0;

	if (NeedsUpdating("light", &lightBufferObj, sizeof(LightBufferType)))
	{
		CheckSuccess(deviceContext->Map(m_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		auto lightBufferPtr = (LightBufferType*)mappedResource.pData;
		lightBufferPtr->ambientColor = lightBufferObj.ambientColor;
		lightBufferPtr->diffuseColor = lightBufferObj.diffuseColor;
		lightBufferPtr->lightDirection = lightBufferObj.lightDirection;
		lightBufferPtr->specularColor = lightBufferObj.specularColor;
		lightBufferPtr->specularPower = lightBufferObj.specularPower;

		deviceContext->PSSetConstantBuffers(bufferNumber, 1, &m_lightBuffer);
		deviceContext->Unmap(m_lightBuffer, 0);

		m_shaderCache["light"] = &lightBufferObj;
	}
	return true;
}
