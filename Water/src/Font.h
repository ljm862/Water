#pragma once
#include <DirectXMath.h>
#include <fstream>
#include "Texture.h"
using namespace DirectX;
using namespace std;
class Font
{
private:
	struct FontType
	{
		float left, right;
		int size;
	};

	// Builds the square we render the text on
	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 texture;
	};

public:
	Font();
	~Font();

	bool Init(ID3D11Device*, ID3D11DeviceContext*, int);
	void Shutdown();

	ID3D11ShaderResourceView* GetTexture();

	void BuildVertexArray(void*, std::string, float, float);
	int GetSentencePixelLength(std::string);
	int GetFontHeight();

private:
	bool LoadFontData(std::string);
	void ReleaseFontData();
	bool LoadTexture(ID3D11Device*, ID3D11DeviceContext*, std::string);
	void ReleaseTexture();

private:
	std::unique_ptr<FontType[]> m_Font;
	std::unique_ptr<Texture> m_Texture;
	float m_fontHeight;
	int m_spaceSize;
};

