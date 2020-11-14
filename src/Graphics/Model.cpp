#include "Model.h"

bool Model::Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* command_list, UINT8* constantBufferBegin)
{
	this->command_list = command_list;
	this->constantBufferDataBegin = constantBufferBegin;

	try
	{
		//Cube
		Vertex cube[] =
		{
			Vertex(-0.5f, 0.5f, -0.5f, 0.0f, 1.0f), //FRONT Bottom Left  - [0]
			Vertex(-0.5f, 1.5f, -0.5f, 0.0f, 0.0f), //FRONT Top Left     - [1]
			Vertex( 0.5f, 1.5f, -0.5f, 1.0f, 0.0f), //FRONT Top Right    - [2]
			Vertex( 0.5f, 0.5f, -0.5f, 1.0f, 1.0f), //FRONT Bottom Right - [3]
			Vertex(-0.5f, 0.5f,  0.5f, 0.0f, 1.0f), //BACK Bottom Left   - [4]
			Vertex(-0.5f, 1.5f,  0.5f, 0.0f, 0.0f), //BACK Top Left      - [5]
			Vertex( 0.5f, 1.5f,  0.5f, 1.0f, 0.0f), //BACK Top Right     - [6]
			Vertex( 0.5f, 0.5f,  0.5f, 1.0f, 1.0f), //BACK Bottom Right  - [7]
		};

		HRESULT hr = vertexBuffer.Initialize(device, command_list, cube, ARRAYSIZE(cube));
		COM_ERROR_IF_FAILED(hr, "Failed to initialize vertex buffer.");

		DWORD indicies[] =
		{
			0, 1, 2, //FRONT
			0, 2, 3, //FRONT
			4, 7, 6, //BACK 
			4, 6, 5, //BACK
			3, 2, 6, //RIGHT SIDE
			3, 6, 7, //RIGHT SIDE
			4, 5, 1, //LEFT SIDE
			4, 1, 0, //LEFT SIDE
			1, 5, 6, //TOP
			1, 6, 2, //TOP
			0, 3, 7, //BOTTOM
			0, 7, 4, //BOTTOM
		};

		hr = indexBuffer.Initialize(device, command_list, indicies, ARRAYSIZE(indicies));
		COM_ERROR_IF_FAILED(hr, "Failed to initialize index buffer.");
	}
	catch (COMException& e)
	{
		ErrorLogger::Log(e);
		return false;
	}

	UpdateWorldMatrix();

	return true;
}

void Model::Render(const XMMATRIX& viewProjMatrix)
{
	constantBufferData.mat = worldMatrix * viewProjMatrix;
	constantBufferData.mat = XMMatrixTranspose(constantBufferData.mat);

	memcpy(constantBufferDataBegin, &constantBufferData, sizeof(constantBufferData));

	command_list->IASetVertexBuffers(0, 1, &vertexBuffer.Get());
	command_list->IASetIndexBuffer(&indexBuffer.Get());

	command_list->DrawIndexedInstanced(indexBuffer.GetIndexCount(), 1, 0, 0, 0);
}

void Model::UpdateWorldMatrix()
{
	worldMatrix = XMMatrixIdentity();
}

void Model::ReleaseExtra()
{
	indexBuffer.FreeUploadResource();
	vertexBuffer.FreeUploadResource();
}