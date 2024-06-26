#include "Model.h"

bool Model::Initialize(const std::string& path,ID3D12Device* device, ID3D12GraphicsCommandList* command_list)
{
	this->command_list = command_list;
	this->device = device;

	try
	{
		if (!LoadModel(path))
			return false;
	}
	catch (COMException& e)
	{
		ErrorLogger::Log(e);
		return false;
	}

	return true;
}

Model::~Model()
{
	device = nullptr;
	command_list = nullptr;
}

void Model::Render()
{
	//Render meshes
	for (int i = 0; i < meshes.size(); i++)
	{
		command_list->IASetVertexBuffers(0, 1, &meshes[i].GetVertexBufferView());
		command_list->IASetIndexBuffer(&meshes[i].GetIndexBufferView());

		command_list->DrawIndexedInstanced(meshes[i].GetIndexCount(), 1, 0, 0, 0);
	}
}

bool Model::LoadModel(const std::string& path)
{
	Assimp::Importer importer;

	const aiScene* pScene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);

	if (pScene == NULL)
		return false;

	ProcessNode(pScene->mRootNode, pScene);
	return true;
}

void Model::ProcessNode(aiNode* node,const aiScene* scene)
{
	for (UINT i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(ProcessMesh(mesh, scene));
	}

	for (UINT i = 0; i < node->mNumChildren; i++)
	{
		ProcessNode(node->mChildren[i], scene);
	}
}

Mesh Model::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
	std::vector<Vertex> vertices;
	std::vector<DWORD> indices;

	//Retrieve vertex data
	for (UINT i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;
		vertex.pos.x = mesh->mVertices[i].x;
		vertex.pos.y = mesh->mVertices[i].y;
		vertex.pos.z = mesh->mVertices[i].z;

		vertex.normal.x = mesh->mNormals[i].x;
		vertex.normal.y = mesh->mNormals[i].y;
		vertex.normal.z = mesh->mNormals[i].z;

		if (mesh->mTextureCoords[0])
		{
			vertex.texCoord.x = (float)mesh->mTextureCoords[0][i].x;
			vertex.texCoord.y = (float)mesh->mTextureCoords[0][i].y;
		}

		vertices.push_back(vertex);
	}

	//Retrieve index data
	for (UINT i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];

		for (UINT j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	return Mesh(device, command_list, vertices, indices);
}

void Model::ReleaseExtra()
{
	for (int i = 0; i < meshes.size(); i++)
	{
		meshes[i].ReleaseLoadingResources();
	}
}