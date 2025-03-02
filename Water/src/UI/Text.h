#pragma once
#include <vector>
#include "Font.h"
#include "../Utils.h"
class Text
{
private:
	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 texture;
	};

public:
	Text();
	~Text();

	bool Init(ID3D11Device*, ID3D11DeviceContext*, int, int, int, Font*, std::string, int, int, XMFLOAT3);
	void Shutdown();
	void Render(ID3D11DeviceContext*);

	int GetIndexCount();
	void SetText(std::string);
	bool UpdateText(ID3D11DeviceContext*, Font*, int, int, XMFLOAT3);
	XMFLOAT4 GetPixelColor();

private:
	bool InitBuffers(ID3D11Device*, ID3D11DeviceContext*, Font*, int, int, XMFLOAT3);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext*);

private:
	ID3D11Buffer* m_vertexBuffer, * m_indexBuffer;
	int m_screenWidth, m_screenHeight, m_maxLength, m_vertexCount, m_indexCount;
	XMFLOAT4 m_pixelColor;
	std::string m_text;
};

