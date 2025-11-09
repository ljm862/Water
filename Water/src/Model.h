#pragma once
#include <d3d11.h>
#include <directxmath.h>
#include <fstream>
#include <vector>
#include <unordered_map>
#include "Texture.h"
using namespace DirectX;
using namespace std;

#define USEOBJ true;

class Model
{
private:
	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 texture;
		XMFLOAT3 normal;

		bool operator==(const VertexType& other) const {
			return position.x == other.position.x &&
				position.y == other.position.y &&
				position.z == other.position.z &&
				texture.x == other.texture.x &&
				texture.y == other.texture.y &&
				normal.x == other.normal.x &&
				normal.y == other.normal.y &&
				normal.z == other.normal.z;
		}
	};

	struct VertexTypeHash {
		size_t operator()(const Model::VertexType& vertex) const {
			return ((hash<float>()(vertex.position.x) ^
				(hash<float>()(vertex.position.y) << 1)) >> 1) ^
				(hash<float>()(vertex.position.z) << 1) ^
				((hash<float>()(vertex.texture.x) ^
					(hash<float>()(vertex.texture.y) << 1)) >> 1) ^
				(hash<float>()(vertex.normal.x) << 1) ^
				((hash<float>()(vertex.normal.y) ^
					(hash<float>()(vertex.normal.z) << 1)) >> 1);
		}
	};

	struct VertexInfo
	{
		float x, y, z;
		float tu, tv;
		float nx, ny, nz;
	};
public:
	Model();
	~Model();

	bool Init(ID3D11Device*, ID3D11DeviceContext*, std::string, std::string);
	void Shutdown();
	void Render(ID3D11DeviceContext*);

	int GetIndexCount();
	void ReleaseModel();
	ID3D11ShaderResourceView* GetTexture();

private:
	bool InitBuffers(ID3D11Device*);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext*);
	bool CreateBuffer(ID3D11Device*, ID3D11Buffer**, D3D11_USAGE, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, const void*, unsigned int, unsigned int);
	bool LoadTexture(ID3D11Device*, ID3D11DeviceContext*, std::string);
	void ReleaseTexture();
	std::vector<std::string> Split(std::string&, std::string&);

#ifdef USEOBJ
	bool LoadModelObj(std::string);
#else
	bool LoadModelV0(std::string); //V1 to read obj files or something
#endif

private:
	int m_vertexCount, m_indexCount;
	ID3D11Buffer* m_vertexBuffer, * m_indexBuffer;
#ifdef USEOBJ
	std::vector<VertexType> m_model;
#else
	std::vector<VertexInfo> m_model;

#endif
	std::unique_ptr<Texture> m_texture;




};

