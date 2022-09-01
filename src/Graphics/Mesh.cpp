#include "Mesh.h"

Mesh::Mesh(ID3D12Device* device, ID3D12GraphicsCommandList* command_list, std::vector<Vertex>& vertices, std::vector<DWORD>& indicies)
{
	HRESULT hr = vertexBuffer.Initialize(device, command_list, vertices.data(), vertices.size());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize vertex buffer.");

	hr = indexBuffer.Initialize(device, command_list, indicies.data(), indicies.size());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize index buffer.");
}

Mesh::Mesh(const Mesh& mesh)
{
	indexBuffer = mesh.indexBuffer;
	vertexBuffer = mesh.vertexBuffer;
}
D3D12_INDEX_BUFFER_VIEW Mesh::GetIndexBufferView()const
{
	return indexBuffer.Get();
}

D3D12_VERTEX_BUFFER_VIEW Mesh::GetVertexBufferView()const
{
	return vertexBuffer.Get();
}

UINT Mesh::GetIndexCount()
{
	return indexBuffer.GetIndexCount();
}

void Mesh::ReleaseLoadingResources()
{
	indexBuffer.FreeUploadResource();
	vertexBuffer.FreeUploadResource();
}