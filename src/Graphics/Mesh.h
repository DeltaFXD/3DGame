#pragma once
#include "Vertex.h"
#include "Buffers/VertexBuffer.h"
#include "Buffers/IndexBuffer.h"
#include "Utility/ErrorLogger.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

class Mesh
{
public:
	Mesh(ID3D12Device* device, ID3D12GraphicsCommandList* command_list, std::vector<Vertex>& vertices, std::vector<DWORD>& indicies);
	Mesh(const Mesh& mesh);
	void Render();
	void ReleaseLoadingResources();
private:
	ID3D12GraphicsCommandList* command_list;

	VertexBuffer<Vertex> vertexBuffer;
	IndexBuffer indexBuffer;
};