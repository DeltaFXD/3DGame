#pragma once
#include <unordered_map>
#include "Model.h"

class ModelManager
{
public:
	ModelManager();
	~ModelManager();

	ModelManager(const ModelManager& rhs) = delete;
	ModelManager& operator=(const ModelManager& rhs) = delete;

	void Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* command_list);

	Model* LoadModel(const std::string& path, const std::string& name);
	Model* GetModel(const std::string& name);
	Model* GetModel(UINT id);

	void ReleaseUploadResources();
private:
	ID3D12Device* m_device = nullptr;
	ID3D12GraphicsCommandList* m_cmd_list = nullptr;

	UINT m_nextID = 0;

	//Unordered map faster for individual access by key
	std::unordered_map<UINT, Model*> m_models;
	std::vector<std::pair<std::string, UINT>> m_lut;
	std::vector<Mesh> m_meshes;
};