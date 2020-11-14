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

	SetPosition(0.0f, 0.0f, 0.0f);
	SetRotation(0.0f, 0.0f, 0.0f);

	UpdateWorldMatrix();

	return true;
}

void Model::Render(const XMMATRIX& viewProjMatrix, ID3D12DescriptorHeap* cbvsrvHeap, const UINT cbvSize)
{
	constantBufferData.mat = worldMatrix * viewProjMatrix;
	constantBufferData.mat = XMMatrixTranspose(constantBufferData.mat);
	const UINT constantBufferSize = static_cast<UINT>(sizeof(CB_VS_vertexshader) + (256 - sizeof(CB_VS_vertexshader) % 256));
	memcpy(constantBufferDataBegin + constantBufferSize, &constantBufferData, sizeof(constantBufferData));

	CD3DX12_GPU_DESCRIPTOR_HANDLE cbvHandle(cbvsrvHeap->GetGPUDescriptorHandleForHeapStart(), 2, cbvSize);
	command_list->SetGraphicsRootDescriptorTable(1, cbvHandle);
	cbvHandle.Offset(cbvSize);

	command_list->IASetVertexBuffers(0, 1, &vertexBuffer.Get());
	command_list->IASetIndexBuffer(&indexBuffer.Get());

	command_list->DrawIndexedInstanced(indexBuffer.GetIndexCount(), 1, 0, 0, 0);
}

void Model::UpdateWorldMatrix()
{
	worldMatrix = XMMatrixRotationRollPitchYaw(rot.x, rot.y, rot.z) * XMMatrixTranslation(pos.x, pos.y, pos.z);
	XMMATRIX vecRotationMatrix = XMMatrixRotationRollPitchYaw(0.0f, rot.y, 0.0f);
	vec_forward = XMVector3TransformCoord(DEFAULT_FORWARD_VECTOR, vecRotationMatrix);
	vec_backward = XMVector3TransformCoord(DEFAULT_BACKWARD_VECTOR, vecRotationMatrix);
	vec_left = XMVector3TransformCoord(DEFAULT_LEFT_VECTOR, vecRotationMatrix);
	vec_right = XMVector3TransformCoord(DEFAULT_RIGHT_VECTOR, vecRotationMatrix);
}

void Model::ReleaseExtra()
{
	indexBuffer.FreeUploadResource();
	vertexBuffer.FreeUploadResource();
}

const XMVECTOR& Model::GetPositionVector() const
{
	return posVector;
}

const XMFLOAT3& Model::GetPositionFloat3() const
{
	return pos;
}

const XMVECTOR& Model::GetRotationVector() const
{
	return rotVector;
}

const XMFLOAT3& Model::GetRotationFloat3() const
{
	return rot;
}

void Model::SetPosition(const XMVECTOR& pos)
{
	XMStoreFloat3(&this->pos, pos);
	posVector = pos;
	UpdateWorldMatrix();
}

void Model::SetPosition(const XMFLOAT3& pos)
{
	this->pos = pos;
	posVector = XMLoadFloat3(&this->pos);
	UpdateWorldMatrix();
}

void Model::SetPosition(float x, float y, float z)
{
	pos = XMFLOAT3(x, y, z);
	posVector = XMLoadFloat3(&pos);
	UpdateWorldMatrix();
}

void Model::AdjustPosition(const XMVECTOR& pos)
{
	posVector += pos;
	XMStoreFloat3(&this->pos, posVector);
	UpdateWorldMatrix();
}

void Model::AdjustPosition(const XMFLOAT3& pos)
{
	this->pos.x += pos.x;
	this->pos.y += pos.y;
	this->pos.z += pos.z;
	XMStoreFloat3(&this->pos, posVector);
	UpdateWorldMatrix();
}

void Model::AdjustPosition(float x, float y, float z)
{
	pos.x += x;
	pos.y += y;
	pos.z += z;
	posVector = XMLoadFloat3(&pos);
	UpdateWorldMatrix();
}

void Model::SetRotation(const XMVECTOR& rot)
{
	XMStoreFloat3(&this->rot, rot);
	rotVector = rot;
	UpdateWorldMatrix();
}

void Model::SetRotation(const XMFLOAT3& rot)
{
	this->rot = rot;
	rotVector = XMLoadFloat3(&this->rot);
	UpdateWorldMatrix();
}

void Model::SetRotation(float x, float y, float z)
{
	rot = XMFLOAT3(x, y, z);
	rotVector = XMLoadFloat3(&rot);
	UpdateWorldMatrix();
}

void Model::AdjustRotation(const XMVECTOR& rot)
{
	rotVector += rot;
	XMStoreFloat3(&this->rot, rotVector);
	UpdateWorldMatrix();
}

void Model::AdjustRotation(const XMFLOAT3& rot)
{
	this->rot.x += rot.x;
	this->rot.y += rot.y;
	this->rot.z += rot.z;
	rotVector = XMLoadFloat3(&this->rot);
	UpdateWorldMatrix();
}

void Model::AdjustRotation(float x, float y, float z)
{
	rot.x += x;
	rot.y += y;
	rot.z += z;
	rotVector = XMLoadFloat3(&rot);
	UpdateWorldMatrix();
}

void Model::SetLookAtPosition(XMFLOAT3 lookAtPos)
{
	//Model cannot look at the same position as it is in
	if (lookAtPos.x == pos.x && lookAtPos.y == pos.y && lookAtPos.z == pos.z)
		return;

	lookAtPos.x = pos.x - lookAtPos.x;
	lookAtPos.y = pos.y - lookAtPos.y;
	lookAtPos.z = pos.z - lookAtPos.z;

	float pitch = 0.0f;
	if (lookAtPos.y != 0.0f)
	{
		const float distance = sqrtf(lookAtPos.x * lookAtPos.x + lookAtPos.z * lookAtPos.z);
		pitch = atanf(lookAtPos.y / distance);
	}
	float yaw = 0.0f;
	if (lookAtPos.x != 0.0f)
	{
		yaw = atanf(lookAtPos.x / lookAtPos.z);
	}
	if (lookAtPos.z > 0)
		yaw += XM_PI;

	SetRotation(pitch, yaw, 0.0f);
}

const XMVECTOR& Model::GetForwardVector()
{
	return vec_forward;
}

const XMVECTOR& Model::GetRightVector()
{
	return vec_right;
}

const XMVECTOR& Model::GetLeftVector()
{
	return vec_left;
}

const XMVECTOR& Model::GetBackwardVector()
{
	return vec_backward;
}