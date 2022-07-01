#include "Camera.h"

Camera::Camera()
{
	pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	posVector = XMLoadFloat3(&pos);
	rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	rotVector = XMLoadFloat3(&rot);
	UpdateViewMatrix();
}

void Camera::SetProjectionValues(float fovD, float aspectRatio, float nearZ, float farZ)
{
	float fovR = (fovD / 360.0f) / XM_2PI;
	projectionMatrix = XMMatrixPerspectiveFovLH(fovR, aspectRatio, nearZ, farZ);
}

const XMMATRIX& Camera::GetViewMatrix() const
{
	return viewMatrix;
}

const XMMATRIX& Camera::GetProjectionMatrix() const
{
	return projectionMatrix;
}

const XMVECTOR& Camera::GetPositionVector() const
{
	return posVector;
}

const XMFLOAT3& Camera::GetPositionFloat3() const
{
	return pos;
}

const XMVECTOR& Camera::GetRotationVector() const
{
	return rotVector;
}

const XMFLOAT3& Camera::GetRotationFloat3() const
{
	return rot;
}

void Camera::SetPosition(const FXMVECTOR& pos)
{
	XMStoreFloat3(&this->pos, pos);
	posVector = pos;
	UpdateViewMatrix();
}

void Camera::SetPosition(const XMFLOAT3& pos)
{
	this->pos = pos;
	posVector = XMLoadFloat3(&this->pos);
	UpdateViewMatrix();
}

void Camera::SetPosition(float x, float y, float z)
{
	pos = XMFLOAT3(x, y, z);
	posVector = XMLoadFloat3(&pos);
	UpdateViewMatrix();
}

void Camera::SetPosition(float y)
{
	pos.y = y;
	posVector = XMLoadFloat3(&pos);
	UpdateViewMatrix();
}

void Camera::AdjustPosition(const FXMVECTOR& pos)
{
	posVector += pos;
	XMStoreFloat3(&this->pos, posVector);
	UpdateViewMatrix();
}

void Camera::AdjustPosition(const XMFLOAT3& pos)
{
	this->pos.x += pos.x;
	this->pos.y += pos.y;
	this->pos.z += pos.z;
	XMStoreFloat3(&this->pos, posVector);
	UpdateViewMatrix();
}

void Camera::AdjustPosition(float x, float y, float z)
{
	pos.x += x;
	pos.y += y;
	pos.z += z;
	posVector = XMLoadFloat3(&pos);
	UpdateViewMatrix();
}

void Camera::SetRotation(const FXMVECTOR& rot)
{
	XMStoreFloat3(&this->rot, rot);
	rotVector = rot;
	UpdateViewMatrix();
}

void Camera::SetRotation(const XMFLOAT3& rot)
{
	this->rot = rot;
	rotVector = XMLoadFloat3(&this->rot);
	UpdateViewMatrix();
}

void Camera::SetRotation(float x, float y, float z)
{
	rot = XMFLOAT3(x, y, z);
	rotVector = XMLoadFloat3(&rot);
	UpdateViewMatrix();
}

void Camera::AdjustRotation(const FXMVECTOR& rot)
{
	rotVector += rot;
	XMStoreFloat3(&this->rot, rotVector);
	UpdateViewMatrix();
}

void Camera::AdjustRotation(const XMFLOAT3& rot)
{
	this->rot.x += rot.x;
	this->rot.y += rot.y;
	this->rot.z += rot.z;
	rotVector = XMLoadFloat3(&this->rot);
	UpdateViewMatrix();
}

void Camera::AdjustRotation(float x, float y, float z)
{
	rot.x += x;
	rot.y += y;
	rot.z += z;
	rotVector = XMLoadFloat3(&rot);
	UpdateViewMatrix();
}

void Camera::SetLookAtPosition(XMFLOAT3 lookAtPos)
{
	//Camera cannot look at the same position as it is in
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

const XMVECTOR& Camera::GetForwardVector()
{
	return vec_forward;
}

const XMVECTOR& Camera::GetRightVector()
{
	return vec_right;
}

const XMVECTOR& Camera::GetLeftVector()
{
	return vec_left;
}

const XMVECTOR& Camera::GetBackwardVector()
{
	return vec_backward;
}

void Camera::UpdateViewMatrix()
{
	//Calculate rotation matrix
	XMMATRIX camRotationM = XMMatrixRotationRollPitchYaw(rot.x, rot.y, rot.z);
	//Calculate unit vector of camera target based off camera forward value transformed by rotation matrix
	XMVECTOR camTarget = XMVector3TransformCoord(DEFAULT_FORWARD_VECTOR, camRotationM);
	//Move camera target by current camera position
	camTarget += posVector;
	//Calculate up direction based on current rotation
	XMVECTOR upDirection = XMVector3TransformCoord(DEFAULT_UP_VECTOR, camRotationM);
	//Rebuild view matrix
	viewMatrix = XMMatrixLookAtLH(posVector, camTarget, upDirection);

	XMMATRIX vecRotationMatrix = XMMatrixRotationRollPitchYaw(0.0f, rot.y, 0.0f);
	vec_forward = XMVector3TransformCoord(DEFAULT_FORWARD_VECTOR, vecRotationMatrix);
	vec_backward = XMVector3TransformCoord(DEFAULT_BACKWARD_VECTOR, vecRotationMatrix);
	vec_left = XMVector3TransformCoord(DEFAULT_LEFT_VECTOR, vecRotationMatrix);
	vec_right = XMVector3TransformCoord(DEFAULT_RIGHT_VECTOR, vecRotationMatrix);
}