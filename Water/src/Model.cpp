#include "Model.h"
Model::Model()
	: m_vertexBuffer(nullptr), m_indexBuffer(nullptr)
{
}
Model::~Model()
{
}

bool Model::Init(ID3D11Device* device, ID3D11DeviceContext* deviceContext, std::string modelFileName)
{
	if (!LoadModelV0(modelFileName)) return false;
	if (!InitBuffers(device)) return false;

	return true;
}

void Model::Shutdown()
{
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

bool Model::InitBuffers(ID3D11Device* device)
{
	std::vector<VertexType> vertices;
	std::vector<unsigned long> indices;
	std::unordered_map<VertexType, unsigned int, VertexTypeHash> foundVertices;

	int curIndex = 0;
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

	m_vertexCount = vertices.size();
	m_indexCount = indices.size();

	if (!CreateBuffer(device, &m_vertexBuffer, D3D11_USAGE_DEFAULT, (sizeof(VertexType) * m_vertexCount), D3D11_BIND_VERTEX_BUFFER, 0, 0, 0, vertices.data(), 0, 0)) return false;
	if (!CreateBuffer(device, &m_indexBuffer, D3D11_USAGE_DEFAULT, (sizeof(unsigned long) * m_indexCount), D3D11_BIND_INDEX_BUFFER, 0, 0, 0, indices.data(), 0, 0)) return false;

	vertices.~vector();
	indices.~vector();
	return true;
}

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
