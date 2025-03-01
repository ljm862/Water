#include "Camera.h"

Camera::Camera() : m_position(0, 0, 0), m_rotation(0, 0, 0)
{
}

Camera::~Camera()
{
}

void Camera::SetPosition(float x, float y, float z)
{
	m_position = XMFLOAT3(x, y, z);
}

void Camera::SetRotation(float x, float y, float z)
{
	m_rotation = XMFLOAT3(x, y, z);
}

XMFLOAT3 Camera::GetPosition()
{
	return m_position;
}

// Should maybe use quaternions?
XMFLOAT3 Camera::GetRotation()
{
	return m_rotation;
}

void Camera::Render()
{
	XMFLOAT3 up(0.0f, 1.0f, 0.0f);
	XMVECTOR upVector = XMLoadFloat3(&up);

	XMVECTOR positionVector = XMLoadFloat3(&m_position);

	XMFLOAT3 lookAt(0.0f, 0.0f, 1.0f);
	XMVECTOR lookAtVector = XMLoadFloat3(&lookAt);

	float pitch, yaw, roll;
	pitch = m_rotation.x * 0.0174532925f;
	yaw = m_rotation.y * 0.0174532925f;
	roll = m_rotation.z * 0.0174532925f;

	XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);
	lookAtVector = XMVector3TransformCoord(lookAtVector, rotationMatrix);
	upVector = XMVector3TransformCoord(upVector, rotationMatrix);

	lookAtVector = XMVectorAdd(positionVector, lookAtVector);
	m_viewMatrix = XMMatrixLookAtLH(positionVector, lookAtVector, upVector);
}

void Camera::GetViewMatrix(XMMATRIX& viewMatrix)
{
	viewMatrix = m_viewMatrix;
}
