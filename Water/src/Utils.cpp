#include "Utils.h"
bool Utils::SetupAndCreateBuffer(ID3D11Device* device, ID3D11Buffer** buffer, D3D11_USAGE usage, unsigned int byteWidth,
	unsigned int bindFlags, unsigned int cpuAccessFlags, unsigned int miscFlags, unsigned int structureByteStride, void* data)
{
	D3D11_BUFFER_DESC tmpBuffer(0);
	tmpBuffer.Usage = usage;
	tmpBuffer.ByteWidth = byteWidth;
	tmpBuffer.BindFlags = bindFlags;
	tmpBuffer.CPUAccessFlags = cpuAccessFlags;
	tmpBuffer.MiscFlags = miscFlags;
	tmpBuffer.StructureByteStride = structureByteStride;

	D3D11_SUBRESOURCE_DATA resData(0);
	resData.pSysMem = data;
	resData.SysMemPitch = 0;
	resData.SysMemSlicePitch = 0;

	return FAILED(device->CreateBuffer(&tmpBuffer, &resData, buffer));
}

bool Utils::SetupAndCreateBuffer(ID3D11Device* device, ID3D11Buffer** buffer, D3D11_USAGE usage, unsigned int byteWidth,
	unsigned int bindFlags, unsigned int cpuAccessFlags, unsigned int miscFlags, unsigned int structureByteStride)
{
	D3D11_BUFFER_DESC tmpBuffer(0);
	tmpBuffer.Usage = usage;
	tmpBuffer.ByteWidth = byteWidth;
	tmpBuffer.BindFlags = bindFlags;
	tmpBuffer.CPUAccessFlags = cpuAccessFlags;
	tmpBuffer.MiscFlags = miscFlags;
	tmpBuffer.StructureByteStride = structureByteStride;

	return FAILED(device->CreateBuffer(&tmpBuffer, NULL, buffer));
}

void Utils::GenerateElementDesc(std::vector<D3D11_INPUT_ELEMENT_DESC>* buffer, unsigned int index, const char* semanticName, unsigned int semanticIndex, DXGI_FORMAT format, unsigned int inputSlot, unsigned int alignedByteOffset, D3D11_INPUT_CLASSIFICATION inputSlotClass, unsigned int instanceDataStepRate)
{
	buffer->at(index).SemanticName = semanticName;
	buffer->at(index).SemanticIndex = semanticIndex;
	buffer->at(index).Format = format;
	buffer->at(index).InputSlot = inputSlot;
	buffer->at(index).AlignedByteOffset = alignedByteOffset;
	buffer->at(index).InputSlotClass = inputSlotClass;
	buffer->at(index).InstanceDataStepRate = instanceDataStepRate;
}
