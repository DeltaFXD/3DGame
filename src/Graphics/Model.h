#pragma once
#include "Mesh.h"
#include "Buffers/ConstantBufferTypes.h"
#include "Buffers/ConstantBuffer.h"

using namespace DirectX;

class Model
{
public:
	~Model();
	bool Initialize(const std::string& path, ID3D12Device* device, ID3D12GraphicsCommandList* command_list);
	void Render();
	void ReleaseExtra();
private:
	std::vector<Mesh> meshes;

	bool LoadModel(const std::string& path);
	void ProcessNode(aiNode* node, const aiScene* scene);
	Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);

	ID3D12GraphicsCommandList* command_list;
	ID3D12Device* device;

	CB_VS_object constantBufferData;
	ConstantBuffer<CB_VS_object> constant_buffer;
};