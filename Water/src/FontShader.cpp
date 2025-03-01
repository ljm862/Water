#include "FontShader.h"

FontShader::FontShader() : m_pixelBuffer(nullptr)
{
}

FontShader::~FontShader()
{
}

std::string FontShader::GetShaderName()
{
	return "FontShader";
}

bool FontShader::Init(ID3D11Device* device, HWND hwnd)
{
	std::wstring vsf = L"src/shaders/font.vs";
	std::vector<wchar_t> vertexShaderFilename(vsf.begin(), vsf.end());
	vertexShaderFilename.push_back(L'\0');

	std::wstring psf = L"src/shaders/font.ps";
	std::vector<wchar_t> pixelShaderFilename(psf.begin(), psf.end());
	pixelShaderFilename.push_back(L'\0');

	return InitShader(device, hwnd, vertexShaderFilename.data(), pixelShaderFilename.data());
}

bool FontShader::Render(ID3D11DeviceContext* deviceContext, int indexCount, XMMATRIX worldMatrix, XMMATRIX viewMatrix,
	XMMATRIX projectionMatrix, ID3D11ShaderResourceView* texture, XMFLOAT4 pixelColor)
{
	if (!SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix, texture, pixelColor)) return false;

	RenderShader(deviceContext, indexCount);
	return true;
}

bool FontShader::InitShader(ID3D11Device* device, HWND hwnd, std::wstring vsFilename, std::wstring psFilename)
{
	ID3D10Blob* errorMessage(0);
	ID3D10Blob* vertexShaderBuffer(0);
	ID3D10Blob* pixelShaderBuffer(0);

	HRESULT result;
	result = D3DCompileFromFile(vsFilename.c_str(), NULL, NULL, "FontVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &vertexShaderBuffer, &errorMessage);
	if (FAILED(result)) return HandleShaderLoadingError(errorMessage, hwnd, vsFilename);
	result = D3DCompileFromFile(psFilename.c_str(), NULL, NULL, "FontPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pixelShaderBuffer, &errorMessage);
	if (FAILED(result)) return HandleShaderLoadingError(errorMessage, hwnd, psFilename);

	CheckSuccess(device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader));
	CheckSuccess(device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader));

	std::vector<D3D11_INPUT_ELEMENT_DESC> polygonLayout(2);

	Utils::GenerateElementDesc(&polygonLayout, 0, "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0);
	Utils::GenerateElementDesc(&polygonLayout, 1, "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0);
	unsigned int numElements = polygonLayout.size();

	CheckSuccess(device->CreateInputLayout(polygonLayout.data(), numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_layout));

	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;
	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;

	if (Utils::SetupAndCreateBuffer(device, &m_matrixBuffer, D3D11_USAGE_DYNAMIC, sizeof(MatrixBufferType), D3D11_BIND_CONSTANT_BUFFER, D3D11_CPU_ACCESS_WRITE, 0, 0)) return false;
	if (Utils::SetupAndCreateBuffer(device, &m_pixelBuffer, D3D11_USAGE_DYNAMIC, sizeof(PixelBufferType), D3D11_BIND_CONSTANT_BUFFER, D3D11_CPU_ACCESS_WRITE, 0, 0)) return false;

	return CreateSamplerDesc(device, D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP, 0.0f, 1, D3D11_COMPARISON_ALWAYS, 0, 0, 0, 0, 0, D3D11_FLOAT32_MAX);
}

void FontShader::ShutdownShader()
{
	Shader::ShutdownShader();
	if (m_pixelBuffer)
	{
		m_pixelBuffer->Release();
		m_pixelBuffer = 0;
	}
}

bool FontShader::SetShaderParameters(ID3D11DeviceContext* deviceContext, XMMATRIX worldMatrix, XMMATRIX viewMatrix,
	XMMATRIX projectionMatrix, ID3D11ShaderResourceView* texture, XMFLOAT4 pixelColor)
{
	worldMatrix = XMMatrixTranspose(worldMatrix);
	viewMatrix = XMMatrixTranspose(viewMatrix);
	projectionMatrix = XMMatrixTranspose(projectionMatrix);


	D3D11_MAPPED_SUBRESOURCE mappedResource;
	unsigned int bufferNumber = 0;

	MatrixBufferType tmpMatrix;
	tmpMatrix.world = worldMatrix;
	tmpMatrix.view = viewMatrix;
	tmpMatrix.projection = projectionMatrix;
	if (NeedsUpdating("model", &tmpMatrix))
	{
		CheckSuccess(deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		auto matrixBufferPtr = (MatrixBufferType*)mappedResource.pData;
		matrixBufferPtr->world = worldMatrix;
		matrixBufferPtr->view = viewMatrix;
		matrixBufferPtr->projection = projectionMatrix;

		deviceContext->Unmap(m_matrixBuffer, 0);

		//Set the cbuff and tex resource
		deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);
		deviceContext->PSSetShaderResources(0, 1, &texture);

		m_shaderCache["model"] = &tmpMatrix;
	}

	PixelBufferType tmpPixel;
	tmpPixel.pixelColor = pixelColor;
	if (NeedsUpdating("pixel", &tmpPixel))
	{
		CheckSuccess(deviceContext->Map(m_pixelBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		auto dataPtr2 = (PixelBufferType*)mappedResource.pData;
		dataPtr2->pixelColor = pixelColor;

		bufferNumber = 0;
		deviceContext->PSSetConstantBuffers(bufferNumber, 1, &m_pixelBuffer);

		deviceContext->Unmap(m_pixelBuffer, 0);
		m_shaderCache["pixel"] = &tmpPixel;
	}

	return true;
}

void FontShader::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
	deviceContext->IASetInputLayout(m_layout);

	deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	deviceContext->PSSetShader(m_pixelShader, NULL, 0);

	deviceContext->PSSetSamplers(0, 1, &m_sampleState);

	deviceContext->DrawIndexed(indexCount, 0, 0);
}
