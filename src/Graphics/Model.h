#pragma once
#include "Mesh.h"
#include "ConstantBufferTypes.h"
#include "Buffers/ConstantBuffer.h"

using namespace DirectX;

class Model
{
public:
	bool Initialize(const std::string& path, ID3D12Device* device, ID3D12GraphicsCommandList* command_list, UINT8* constantBufferBegin);
	void Render(const XMMATRIX& worldMatrix, const XMMATRIX& viewProjMatrix, ID3D12DescriptorHeap* cbvsrvHeap, const UINT cbvSize);
	void ReleaseExtra();
private:
	std::vector<Mesh> meshes;

	bool LoadModel(const std::string& path);
	void ProcessNode(aiNode* node, const aiScene* scene);
	Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);

	ID3D12GraphicsCommandList* command_list;
	ID3D12Device* device;

	CB_VS_vertexshader constantBufferData;
	UINT8* constantBufferDataBegin;
};