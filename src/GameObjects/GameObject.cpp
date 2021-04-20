#include "GameObject.h"

bool GameObject::Initialize(const std::string& path, ID3D12Device* device, ID3D12GraphicsCommandList* command_list, ConstantBuffer<CB_VS_vertexshader>* constant_buffer)
{
	if (!model.Initialize(path, device, command_list, constant_buffer))
		return false;

	SetPosition(0.0f, 0.0f, 0.0f);
	SetRotation(0.0f, 0.0f, 0.0f);

	UpdateWorldMatrix();

	return true;
}

void GameObject::Render(const XMMATRIX& viewProjMatrix)
{
	model.Render(worldMatrix, viewProjMatrix);
}

void GameObject::ReleaseCreationResources()
{
	model.ReleaseExtra();
}

void GameObject::UpdateWorldMatrix()
{
	worldMatrix = XMMatrixRotationRollPitchYaw(rot.x, rot.y, rot.z) * XMMatrixTranslation(pos.x, pos.y, pos.z);
	XMMATRIX vecRotationMatrix = XMMatrixRotationRollPitchYaw(0.0f, rot.y, 0.0f);
	vec_forward = XMVector3TransformCoord(DEFAULT_FORWARD_VECTOR, vecRotationMatrix);
	vec_backward = XMVector3TransformCoord(DEFAULT_BACKWARD_VECTOR, vecRotationMatrix);
	vec_left = XMVector3TransformCoord(DEFAULT_LEFT_VECTOR, vecRotationMatrix);
	vec_right = XMVector3TransformCoord(DEFAULT_RIGHT_VECTOR, vecRotationMatrix);
}

const XMVECTOR& GameObject::GetPositionVector() const
{
	return posVector;
}

const XMFLOAT3& GameObject::GetPositionFloat3() const
{
	return pos;
}

const XMVECTOR& GameObject::GetRotationVector() const
{
	return rotVector;
}

const XMFLOAT3& GameObject::GetRotationFloat3() const
{
	return rot;
}

void GameObject::SetPosition(const XMVECTOR& pos)
{
	XMStoreFloat3(&this->pos, pos);
	posVector = pos;
	UpdateWorldMatrix();
}

void GameObject::SetPosition(const XMFLOAT3& pos)
{
	this->pos = pos;
	posVector = XMLoadFloat3(&this->pos);
	UpdateWorldMatrix();
}

void GameObject::SetPosition(float x, float y, float z)
{
	pos = XMFLOAT3(x, y, z);
	posVector = XMLoadFloat3(&pos);
	UpdateWorldMatrix();
}

void GameObject::SetPosition(float y)
{
	pos.y = y;
	posVector = XMLoadFloat3(&pos);
	UpdateWorldMatrix();
}

void GameObject::AdjustPosition(const XMVECTOR& pos)
{
	posVector += pos;
	XMStoreFloat3(&this->pos, posVector);
	UpdateWorldMatrix();
}

void GameObject::AdjustPosition(const XMFLOAT3& pos)
{
	this->pos.x += pos.x;
	this->pos.y += pos.y;
	this->pos.z += pos.z;
	XMStoreFloat3(&this->pos, posVector);
	UpdateWorldMatrix();
}

void GameObject::AdjustPosition(float x, float y, float z)
{
	pos.x += x;
	pos.y += y;
	pos.z += z;
	posVector = XMLoadFloat3(&pos);
	UpdateWorldMatrix();
}

void GameObject::SetRotation(const XMVECTOR& rot)
{
	XMStoreFloat3(&this->rot, rot);
	rotVector = rot;
	UpdateWorldMatrix();
}

void GameObject::SetRotation(const XMFLOAT3& rot)
{
	this->rot = rot;
	rotVector = XMLoadFloat3(&this->rot);
	UpdateWorldMatrix();
}

void GameObject::SetRotation(float x, float y, float z)
{
	rot = XMFLOAT3(x, y, z);
	rotVector = XMLoadFloat3(&rot);
	UpdateWorldMatrix();
}

void GameObject::AdjustRotation(const XMVECTOR& rot)
{
	rotVector += rot;
	XMStoreFloat3(&this->rot, rotVector);
	UpdateWorldMatrix();
}

void GameObject::AdjustRotation(const XMFLOAT3& rot)
{
	this->rot.x += rot.x;
	this->rot.y += rot.y;
	this->rot.z += rot.z;
	rotVector = XMLoadFloat3(&this->rot);
	UpdateWorldMatrix();
}

void GameObject::AdjustRotation(float x, float y, float z)
{
	rot.x += x;
	rot.y += y;
	rot.z += z;
	rotVector = XMLoadFloat3(&rot);
	UpdateWorldMatrix();
}

void GameObject::SetLookAtPosition(XMFLOAT3 lookAtPos)
{
	//GameObject cannot look at the same position as it is in
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

const XMVECTOR& GameObject::GetForwardVector()
{
	return vec_forward;
}

const XMVECTOR& GameObject::GetRightVector()
{
	return vec_right;
}

const XMVECTOR& GameObject::GetLeftVector()
{
	return vec_left;
}

const XMVECTOR& GameObject::GetBackwardVector()
{
	return vec_backward;
}