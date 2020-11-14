#pragma once
#include "Vertex.h"
#include "ConstantBufferTypes.h"
#include "Buffers/VertexBuffer.h"
#include "Buffers/IndexBuffer.h"
#include "Utility/ErrorLogger.h"

using namespace DirectX;

class Model
{
public:
	bool Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* command_list, UINT8* constantBufferBegin);
	void Render(const XMMATRIX& viewProjMatrix);
	void ReleaseExtra();
private:
	void UpdateWorldMatrix();

	ID3D12GraphicsCommandList* command_list;
	UINT8* constantBufferDataBegin;

	CB_VS_vertexshader constantBufferData;
	VertexBuffer<Vertex> vertexBuffer;
	IndexBuffer indexBuffer;

	XMMATRIX worldMatrix = XMMatrixIdentity();
};