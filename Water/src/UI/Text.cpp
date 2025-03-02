#include "Text.h"

Text::Text() : m_vertexBuffer(nullptr), m_indexBuffer(nullptr)
{
}

Text::~Text()
{
}

bool Text::Init(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int screenWidth, int screenHeight, int maxLength, Font* font, std::string text,
	int positionX, int positionY, XMFLOAT3 color)
{
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;

	m_maxLength = maxLength;
	SetText(text);
	return InitBuffers(device, deviceContext, font, positionX, positionY, color);
}

void Text::Shutdown()
{
	ShutdownBuffers();
}

void Text::Render(ID3D11DeviceContext* deviceContext)
{
	RenderBuffers(deviceContext);
}

int Text::GetIndexCount()
{
	return m_indexCount;
}

void Text::SetText(std::string text)
{
	// Can have a "debug text" which is this class but with a prefix of FPS: or Frame: and then we just update the value each call
	m_text = text;
}

bool Text::UpdateText(ID3D11DeviceContext* deviceContext, Font* font, int positionX, int positionY, XMFLOAT3 color)
{
	m_pixelColor = XMFLOAT4(color.x, color.y, color.z, 1.0f);

	int numLetters = m_text.length();
	if (numLetters > m_maxLength) return false;

	//auto vertices = make_unique<VertexType[]>(m_vertexCount);
	auto vertices = new VertexType[m_vertexCount];
	memset(vertices, 0, (sizeof(VertexType) * m_vertexCount));

	float drawX = (float)(((m_screenWidth / 2) * -1) + positionX);
	float drawY = (float)((m_screenHeight / 2) - positionY);

	font->BuildVertexArray((void*)vertices, m_text, drawX, drawY);

	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	result = deviceContext->Map(m_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result)) return false;

	auto verticesPtr = (VertexType*)mappedResource.pData;

	memcpy(verticesPtr, (void*)vertices, (sizeof(VertexType) * m_vertexCount));

	deviceContext->Unmap(m_vertexBuffer, 0);

	//vertices.reset();
	delete[] vertices;
	vertices = nullptr;

	return true;
}

XMFLOAT4 Text::GetPixelColor()
{
	return m_pixelColor;
}

bool Text::InitBuffers(ID3D11Device* device, ID3D11DeviceContext* deviceContext, Font* font, int positionX, int positionY, XMFLOAT3 color)
{
	const int verticesPerSquare = 4;
	const int indicesPerSquare = 6;
	m_vertexCount = verticesPerSquare * m_maxLength;
	m_indexCount = indicesPerSquare * m_maxLength;

	auto vertices = make_unique<VertexType[]>(m_vertexCount);
	auto indices = make_unique<unsigned long[]>(m_indexCount);

	for (int i = 0; i < m_maxLength; i++)
	{
		indices.get()[0 + (indicesPerSquare * i)] = 0 + (verticesPerSquare * i);
		indices.get()[1 + (indicesPerSquare * i)] = 1 + (verticesPerSquare * i);
		indices.get()[2 + (indicesPerSquare * i)] = 2 + (verticesPerSquare * i);
		indices.get()[3 + (indicesPerSquare * i)] = 1 + (verticesPerSquare * i);
		indices.get()[4 + (indicesPerSquare * i)] = 0 + (verticesPerSquare * i);
		indices.get()[5 + (indicesPerSquare * i)] = 3 + (verticesPerSquare * i);
	}

	if (Utils::SetupAndCreateBuffer(device, &m_vertexBuffer, D3D11_USAGE_DYNAMIC, sizeof(VertexType) * m_vertexCount, D3D11_BIND_VERTEX_BUFFER, D3D11_CPU_ACCESS_WRITE, 0, 0, vertices.get())) return false;
	if (Utils::SetupAndCreateBuffer(device, &m_indexBuffer, D3D11_USAGE_DEFAULT, sizeof(unsigned long) * m_indexCount, D3D11_BIND_INDEX_BUFFER, 0, 0, 0, indices.get())) return false;

	vertices.reset();
	indices.reset();

	return UpdateText(deviceContext, font, positionX, positionY, color);
}

void Text::ShutdownBuffers()
{
	if (m_indexBuffer)
	{
		m_indexBuffer->Release();
		m_indexBuffer = nullptr;
	}

	if (m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = nullptr;
	}
}

void Text::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	unsigned int stride, offset;
	stride = sizeof(VertexType);
	offset = 0;

	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
