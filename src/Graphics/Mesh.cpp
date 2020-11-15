#include "Mesh.h"

Mesh::Mesh(ID3D12Device* device, ID3D12GraphicsCommandList* command_list, std::vector<Vertex>& vertices, std::vector<DWORD>& indicies)
{
	this->command_list = command_list;

	HRESULT hr = vertexBuffer.Initialize(device, command_list, vertices.data(), vertices.size());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize vertex buffer.");

	hr = indexBuffer.Initialize(device, command_list, indicies.data(), indicies.size());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize index buffer.");
}

Mesh::Mesh(const Mesh& mesh)
{
	command_list = mesh.command_list;
	indexBuffer = mesh.indexBuffer;
	vertexBuffer = mesh.vertexBuffer;
}

void Mesh::Render()
{
	command_list->IASetVertexBuffers(0, 1, &vertexBuffer.Get());
	command_list->IASetIndexBuffer(&indexBuffer.Get());

	command_list->DrawIndexedInstanced(indexBuffer.GetIndexCount(), 1, 0, 0, 0);
}

void Mesh::ReleaseLoadingResources()
{
	indexBuffer.FreeUploadResource();
	vertexBuffer.FreeUploadResource();
}