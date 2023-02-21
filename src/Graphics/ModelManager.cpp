#include "ModelManager.h"

ModelManager::ModelManager()
{

}

ModelManager::~ModelManager()
{
	for (auto it = m_models.begin(); it != m_models.end(); it++)
	{
		delete it->second;
	}

	m_device = nullptr;
	m_cmd_list = nullptr;
}

void ModelManager::Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* command_list)
{
	m_device = device;
	m_cmd_list = command_list;
}

Model* ModelManager::LoadModel(const std::string& path, const std::string& name)
{
	Model* model = nullptr;

	return model;
}

Model* ModelManager::GetModel(const std::string& name)
{
	Model* result = nullptr;

	for (auto it = m_lut.begin(); it != m_lut.end(); it++)
	{
		if (it->first.compare(name) == 0)
		{
			result = m_models[it->second];
			break;
		}
	}

	return result;
}

Model* ModelManager::GetModel(UINT id)
{
	Model* result = nullptr;

	auto it = m_models.find(id);

	if (it != m_models.end())
		result = m_models[id];

	return result;
}

void ModelManager::ReleaseUploadResources()
{
	for (auto it = m_meshes.begin(); it != m_meshes.end(); it++)
	{
		it->ReleaseLoadingResources();
	}

	m_meshes.clear();
}