#include "Font.h"

Font::Font() : m_Font(nullptr), m_Texture(nullptr)
{
}

Font::~Font()
{
}

bool Font::Init(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int fontChoice)
{
	char fontFilename[128];
	char fontTextureFilename[128];
	switch (fontChoice)
	{

	case 0:
	default:
	{
		strcpy_s(fontFilename, "src/resources/font/font01.txt");
		strcpy_s(fontTextureFilename, "src/resources/font/font01.tga");
		m_fontHeight = 32.0f;
		m_spaceSize = 3;
		break;
	}
	}

	if (!LoadFontData(fontFilename)) return false;
	if (!LoadTexture(device, deviceContext, fontTextureFilename)) return false;

	return true;
	return false;
}

void Font::Shutdown()
{
	ReleaseTexture();
	ReleaseFontData();
}

ID3D11ShaderResourceView* Font::GetTexture()
{
	return m_Texture->GetTexture();
}

void Font::BuildVertexArray(void* vertices, std::string sentence, float drawX, float drawY)
{
	int numLetters = sentence.length();
	auto verticesArray = (VertexType*)vertices;

	int index = 0;
	int letter;

	for (int i = 0; i < numLetters; i++)
	{
		letter = ((int)sentence[i]) - 32;

		if (letter == 0)
		{
			drawX += m_spaceSize;
			continue;
		}

		verticesArray[index].position = XMFLOAT3(drawX, drawY, 0.0f);  // Top left.
		verticesArray[index].texture = XMFLOAT2(m_Font[letter].left, 0.0f);
		index++;

		verticesArray[index].position = XMFLOAT3((drawX + m_Font[letter].size), (drawY - m_fontHeight), 0.0f);  // Bottom right.
		verticesArray[index].texture = XMFLOAT2(m_Font[letter].right, 1.0f);
		index++;

		verticesArray[index].position = XMFLOAT3(drawX, (drawY - m_fontHeight), 0.0f);  // Bottom left.
		verticesArray[index].texture = XMFLOAT2(m_Font[letter].left, 1.0f);
		index++;

		verticesArray[index].position = XMFLOAT3(drawX + m_Font[letter].size, drawY, 0.0f);  // Top right.
		verticesArray[index].texture = XMFLOAT2(m_Font[letter].right, 0.0f);
		index++;

		drawX = drawX + m_Font[letter].size + 1.0f;
	}
}

int Font::GetSentencePixelLength(std::string sentence)
{
	int numLetters = sentence.length();
	int pixelLength = 0;
	int letter;
	for (int i = 0; i < numLetters; i++)
	{
		letter = ((int)sentence[i]) - 32;
		if (letter == 0)
		{
			pixelLength += m_spaceSize;
			continue;
		}

		pixelLength += (m_Font[letter].size + 1);

	}
	return pixelLength;
}

int Font::GetFontHeight()
{
	return (int)m_fontHeight;
}

bool Font::LoadFontData(std::string filename)
{
	ifstream fin;
	const int font_count = 95;
	m_Font = make_unique<FontType[]>(font_count);

	fin.open(filename);
	if (fin.fail()) return false;

	char temp;
	for (int i = 0; i < font_count; i++)
	{
		// We only need the coordinates and size
		fin.get(temp);
		while (temp != ' ')
		{
			fin.get(temp);
		}
		fin.get(temp);
		while (temp != ' ')
		{
			fin.get(temp);
		}

		fin >> m_Font[i].left;
		fin >> m_Font[i].right;
		fin >> m_Font[i].size;
	}
	fin.close();
	return true;
}

void Font::ReleaseFontData()
{
	m_Font.reset();
}

bool Font::LoadTexture(ID3D11Device* device, ID3D11DeviceContext* deviceContext, std::string filename)
{
	m_Texture = make_unique<Texture>();
	return m_Texture->Init(device, deviceContext, filename);
}

void Font::ReleaseTexture()
{
	m_Texture.reset();
}
