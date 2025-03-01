#include "Texture.h"

Texture::Texture() : m_texture(nullptr), m_textureView(nullptr), m_targaData(nullptr)
{
}

Texture::~Texture()
{
}

bool Texture::Init(ID3D11Device* device, ID3D11DeviceContext* deviceContext, std::string filename)
{
	if (!LoadTargaFile(filename)) return false;

	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Height = m_height;
	textureDesc.Width = m_width;
	textureDesc.MipLevels = 0;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	CheckSuccess(device->CreateTexture2D(&textureDesc, nullptr, &m_texture));

	auto rowPitch = (m_width * 4) * sizeof(unsigned char);
	deviceContext->UpdateSubresource(m_texture, 0, nullptr, m_targaData, rowPitch, 0);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;

	CheckSuccess(device->CreateShaderResourceView(m_texture, &srvDesc, &m_textureView));
	deviceContext->GenerateMips(m_textureView);

	delete[] m_targaData;
	m_targaData = nullptr;

	return true;
}

void Texture::Shutdown()
{
	if (m_texture)
	{
		m_texture->Release();
		m_texture = nullptr;
	}

	if (m_textureView)
	{
		m_textureView->Release();
		m_textureView = nullptr;
	}

	if (m_targaData)
	{
		delete m_targaData;
		m_targaData = nullptr;
	}
}
bool Texture::LoadTargaFile(std::string filename)
{
	int error, bpp;
	FILE* filePtr;
	unsigned int count;
	TargaHeader targaFileHeader;

	error = fopen_s(&filePtr, filename.c_str(), "rb");
	if (error != 0) return false;

	count = (unsigned int)fread(&targaFileHeader, sizeof(TargaHeader), 1, filePtr);
	if (count != 1) return false;

	m_height = (int)targaFileHeader.height;
	m_width = (int)targaFileHeader.width;
	bpp = (int)targaFileHeader.bpp;

	// We only support 32 bit targa files
	if (bpp == 32) return LoadTarga32Bit(filePtr);
	return false;
}

bool Texture::LoadTarga32Bit(FILE* filePtr)
{
	int error, imageSize, index, i, j, k;
	unsigned int count;
	unsigned char* targaImage;

	imageSize = m_width * m_height * 4; // 4 bytes per pixel (32 bit)

	targaImage = new unsigned char[imageSize];

	count = (unsigned int)fread(targaImage, 1, imageSize, filePtr);
	if (count != imageSize) return false;

	error = fclose(filePtr);
	if (error != 0) return false;

	m_targaData = new unsigned char[imageSize];

	index = 0;
	k = imageSize - (m_width * 4);
	for (j = 0; j < m_height; j++)
	{
		for (i = 0; i < m_width; i++)
		{
			m_targaData[index + 0] = targaImage[k + 2]; //red
			m_targaData[index + 1] = targaImage[k + 1]; //green
			m_targaData[index + 2] = targaImage[k + 0]; //blue
			m_targaData[index + 3] = targaImage[k + 3]; //alpha

			k += 4;
			index += 4;
		}
		k -= (m_width * 8);
	}
	delete[] targaImage;
	targaImage = 0;

	return true;
}

void Texture::CheckSuccess(HRESULT hResult)
{
	if (FAILED(hResult))
	{
		throw "Failed to load texture";
	}
}

ID3D11ShaderResourceView* Texture::GetTexture()
{
	return m_textureView;
}

int Texture::GetWidth()
{
	return m_width;
}

int Texture::GetHeight()
{
	return m_height;
}
