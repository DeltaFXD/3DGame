#pragma once
#include "Mesh.h"
#include "Buffers/ConstantBufferTypes.h"
#include "Buffers/ConstantBuffer.h"

using namespace DirectX;

class Model
{
public:
	bool Initialize(const std::string& path, ID3D12Device* device, ID3D12GraphicsCommandList* command_list, ConstantBuffer<CB_VS_vertexshader>* constant_buffer);
	void Render(const XMMATRIX& worldMatrix, const XMMATRIX& viewProjMatrix);
	void ReleaseExtra();
private:
	std::vector<Mesh> meshes;

	bool LoadModel(const std::string& path);
	void ProcessNode(aiNode* node, const aiScene* scene);
	Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);

	ID3D12GraphicsCommandList* command_list;
	ID3D12Device* device;

	CB_VS_vertexshader constantBufferData;
	ConstantBuffer<CB_VS_vertexshader>* constant_buffer;
};