#pragma once
#include <d3d11.h>
#include <stdio.h>
#include <string>
class Texture
{
private:
	struct TargaHeader
	{
		unsigned char data1[12];
		unsigned short width;
		unsigned short height;
		unsigned char bpp;
		unsigned char data2;
	};

public:
	Texture();
	~Texture();

	bool Init(ID3D11Device*, ID3D11DeviceContext*, std::string);
	void Shutdown();

	ID3D11ShaderResourceView* GetTexture();

	int GetWidth();
	int GetHeight();

private:
	bool LoadTargaFile(std::string);
	bool LoadTarga32Bit(FILE* filePtr);
	void CheckSuccess(HRESULT);

private:
	unsigned char* m_targaData;
	ID3D11Texture2D* m_texture;
	ID3D11ShaderResourceView* m_textureView;
	int m_width, m_height;
};

