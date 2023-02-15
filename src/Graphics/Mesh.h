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
	void ReleaseLoadingResources();
	D3D12_INDEX_BUFFER_VIEW& GetIndexBufferView();
	D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView();
	UINT GetIndexCount();
private:
	VertexBuffer<Vertex> vertexBuffer;
	IndexBuffer indexBuffer;
};