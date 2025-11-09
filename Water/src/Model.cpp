#include "Model.h"
Model::Model()
	: m_vertexBuffer(nullptr), m_indexBuffer(nullptr)
{
}
Model::~Model()
{
}

bool Model::Init(ID3D11Device* device, ID3D11DeviceContext* deviceContext, std::string modelFileName, std::string textureFileName)
{
#ifdef USEOBJ
	if (!LoadModelObj(modelFileName)) return false;
#else
	if (!LoadModelV0(modelFileName)) return false;
#endif
	if (!InitBuffers(device)) return false;
	if (!LoadTexture(device, deviceContext, textureFileName)) return false;

	return true;
}

void Model::Shutdown()
{
	ReleaseTexture();
	ShutdownBuffers();
	ReleaseModel();
}

void Model::Render(ID3D11DeviceContext* deviceContext)
{
	RenderBuffers(deviceContext);
}

int Model::GetIndexCount()
{
	return m_indexCount;
}

void Model::ShutdownBuffers()
{
	if (m_indexBuffer)
	{
		m_indexBuffer->Release();
		m_indexBuffer = 0;
	}

	if (m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = 0;
	}
}

void Model::ReleaseModel()
{
	m_model.clear();
}

ID3D11ShaderResourceView* Model::GetTexture()
{
	return m_texture->GetTexture();
}

bool Model::InitBuffers(ID3D11Device* device)
{
	std::vector<VertexType> vertices;
	std::vector<unsigned long> indices;
	std::unordered_map<VertexType, unsigned int, VertexTypeHash> foundVertices;
	int curIndex = 0;

#ifdef USEOBJ
	for (int i = 0; i < m_model.size(); i++)
	{
		auto tmpVertex = m_model[i];
		if (foundVertices.count(tmpVertex) == 0)
		{
			foundVertices[tmpVertex] = curIndex;
			vertices.push_back(tmpVertex);
			indices.push_back(curIndex);
			curIndex++;
			continue;
		}
		indices.push_back(foundVertices[tmpVertex]);
	}
#else

	for (int i = 0; i < m_vertexCount; i++)
	{
		VertexType tmpVertex;
		tmpVertex.position = XMFLOAT3(m_model[i].x, m_model[i].y, m_model[i].z);
		tmpVertex.texture = XMFLOAT2(m_model[i].tu, m_model[i].tv);
		tmpVertex.normal = XMFLOAT3(m_model[i].nx, m_model[i].ny, m_model[i].nz);

		if (foundVertices.count(tmpVertex) == 0)
		{
			foundVertices[tmpVertex] = curIndex;
			vertices.push_back(tmpVertex);
			indices.push_back(curIndex);
			curIndex++;
			continue;
		}

		indices.push_back(foundVertices[tmpVertex]);
	}
#endif
	m_vertexCount = vertices.size();
	m_indexCount = indices.size();

	if (!CreateBuffer(device, &m_vertexBuffer, D3D11_USAGE_DEFAULT, (sizeof(VertexType) * m_vertexCount), D3D11_BIND_VERTEX_BUFFER, 0, 0, 0, vertices.data(), 0, 0)) return false;
	if (!CreateBuffer(device, &m_indexBuffer, D3D11_USAGE_DEFAULT, (sizeof(unsigned long) * m_indexCount), D3D11_BIND_INDEX_BUFFER, 0, 0, 0, indices.data(), 0, 0)) return false;

	vertices.~vector();
	indices.~vector();

	return true;
}

#ifndef USEOBJ
bool Model::LoadModelV0(std::string modelFileName)
{
	ifstream fin;
	char input;

	fin.open(modelFileName);
	if (fin.fail()) return false;

	fin.get(input);
	while (input != ':')
	{
		fin.get(input);
	}
	fin >> m_vertexCount;

	fin.get(input);
	while (input != ':')
	{
		fin.get(input);
	}
	fin.get(input);
	fin.get(input);

	// Vertex Data Reading
	for (int i = 0; i < m_vertexCount; i++)
	{
		VertexInfo vertex;
		fin >> vertex.x >> vertex.y >> vertex.z;
		fin >> vertex.tu >> vertex.tv;
		fin >> vertex.nx >> vertex.ny >> vertex.nz;
		m_model.push_back(vertex);
	}
	fin.close();

	return true;
}
#else
bool Model::LoadModelObj(std::string modelFilename)
{
	ifstream fin;

	fin.open(modelFilename);
	if (fin.fail()) return false;
	std::unordered_map<VertexType, unsigned int, VertexTypeHash> foundVertices;
	std::string line;
	std::vector<XMFLOAT3> vertices;
	std::vector<XMFLOAT2> uvs;
	std::vector<XMFLOAT3> normals;
	std::string del = "/";

	while (!fin.eof())
	{
		fin >> line;
		if (line == "v")
		{
			float val1;
			float val2;
			float val3;
			fin >> val1;
			fin >> val2;
			fin >> val3;
			auto vertex = XMFLOAT3(val1, val2, val3);
			vertices.push_back(vertex);
		}
		if (line == "vt")
		{
			float val1;
			float val2;
			fin >> val1;
			fin >> val2;
			auto uv = XMFLOAT2(val1, val2);
			uvs.push_back(uv);
		}
		if (line == "vn")
		{
			float val1;
			float val2;
			float val3;
			fin >> val1;
			fin >> val2;
			fin >> val3;
			auto normal = XMFLOAT3(val1, val2, val3);
			normals.push_back(normal);
		}
		if (line == "f")
		{
			for (int i = 0; i < 3; i++)
			{
				VertexType v;
				fin >> line;
				auto splits = Split(line, del);

				auto pos = std::stoi(splits[0]);
				auto tex = std::stoi(splits[1]);
				auto nrm = std::stoi(splits[2]);
				v.position = vertices[pos - 1];
				v.texture = uvs[tex - 1];
				v.normal = normals[nrm - 1];
				m_model.push_back(v);
			}
		}
		continue;
	}
	fin.close();
	return true;
}
#endif
void Model::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	unsigned int stride = sizeof(VertexType);
	unsigned int offset = 0;

	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

bool Model::CreateBuffer(ID3D11Device* device, ID3D11Buffer** memberBuffer, D3D11_USAGE usage, unsigned int byteWidth, unsigned int byteFlags, unsigned int cpuAccessFlags, unsigned int miscFlags, unsigned int structureByteStride, const void* inputData, unsigned int sysMemPitch, unsigned int sysMemSlicePitch)
{
	D3D11_BUFFER_DESC buffer;
	D3D11_SUBRESOURCE_DATA data;
	buffer.Usage = usage;
	buffer.ByteWidth = byteWidth;
	buffer.BindFlags = byteFlags;
	buffer.CPUAccessFlags = cpuAccessFlags;
	buffer.MiscFlags = miscFlags;
	buffer.StructureByteStride = structureByteStride;

	data.pSysMem = inputData;
	data.SysMemPitch = sysMemPitch;
	data.SysMemSlicePitch = sysMemSlicePitch;

	return !FAILED(device->CreateBuffer(&buffer, &data, memberBuffer));
}

bool Model::LoadTexture(ID3D11Device* device, ID3D11DeviceContext* deviceContext, std::string filename)
{
	m_texture = std::make_unique<Texture>();
	return m_texture->Init(device, deviceContext, filename);
}

void Model::ReleaseTexture()
{
	if (m_texture)
	{
		m_texture->Shutdown();
	}

	m_texture.reset();
}

std::vector<std::string> Model::Split(std::string& s, std::string& delimiter) {
	std::vector<std::string> tokens;
	size_t pos = 0;
	std::string token;
	while ((pos = s.find(delimiter)) != std::string::npos) {
		token = s.substr(0, pos);
		tokens.push_back(token);
		s.erase(0, pos + delimiter.length());
	}
	tokens.push_back(s);

	return tokens;
}