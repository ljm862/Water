#pragma once
#include <d3d11.h>
#include <vector>
class Utils
{
public:
	static bool SetupAndCreateBuffer(ID3D11Device* device, ID3D11Buffer** buffer, D3D11_USAGE usage, unsigned int byteWidth,
		unsigned int bindFlags, unsigned int cpuAccessFlags, unsigned int miscFlags, unsigned int structureByteStride, void* data);

	static bool SetupAndCreateBuffer(ID3D11Device* device, ID3D11Buffer** buffer, D3D11_USAGE usage, unsigned int byteWidth,
		unsigned int bindFlags, unsigned int cpuAccessFlags, unsigned int miscFlags, unsigned int structureByteStride);

	static void GenerateElementDesc(std::vector<D3D11_INPUT_ELEMENT_DESC>* buffer, unsigned int index, const char* semanticName,
		unsigned int semanticIndex, DXGI_FORMAT format, unsigned int inputSlot, unsigned int alignedByteOffset,
		D3D11_INPUT_CLASSIFICATION inputSlotClass, unsigned int instanceDataStepRate);

};

