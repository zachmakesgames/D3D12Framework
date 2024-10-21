#include "Camera.h"

Camera::Camera() :
	mPosition({ 0, 0, 0 }), mYaw(0), mPitch(0)
{
}

DirectX::XMFLOAT4X4 Camera::GetViewMatrix()
{
	DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(mPosition.x, mPosition.y, mPosition.z);
	DirectX::XMMATRIX rotationYaw = DirectX::XMMatrixRotationY(mYaw);
	DirectX::XMMATRIX rotationPitch = DirectX::XMMatrixRotationX(mPitch);

	DirectX::XMMATRIX rotation = DirectX::XMMatrixMultiply(rotationYaw, rotationPitch);

	DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixMultiply(translation, rotation);

	DirectX::XMStoreFloat4x4(&mViewMatrix, DirectX::XMMatrixTranspose(viewMatrix));

	return mViewMatrix;
}

void Camera::SetPosition(DirectX::XMFLOAT3 newPosition)
{
	mPosition = newPosition;
}

void Camera::SetPosition(DirectX::XMVECTOR newPositionV)
{
	DirectX::XMStoreFloat3(&mPosition, newPositionV);
}

void Camera::AddPosition(DirectX::XMFLOAT3 position)
{
	DirectX::XMVECTOR currentPosV = DirectX::XMLoadFloat3(&mPosition);
	DirectX::XMVECTOR positionV = DirectX::XMLoadFloat3(&position);
	DirectX::XMVECTOR newPosV = DirectX::XMVectorAdd(currentPosV, positionV);

	DirectX::XMStoreFloat3(&mPosition, newPosV);
}

void Camera::AddPosition(DirectX::XMVECTOR positionV)
{
	DirectX::XMVECTOR currentPosV = DirectX::XMLoadFloat3(&mPosition);
	DirectX::XMVECTOR newPosV = DirectX::XMVectorAdd(currentPosV, positionV);

	DirectX::XMStoreFloat3(&mPosition, newPosV);
}

void Camera::SetYaw(float newYaw)
{
	mYaw = newYaw;
}

void Camera::SetPitch(float newPitch)
{
	mPitch = newPitch;
}

void Camera::AddYaw(float yaw)
{
	mYaw += yaw;
}

void Camera::AddPitch(float pitch)
{	
	mPitch += pitch;
}

DirectX::XMFLOAT3 Camera::GetForwardVector()
{
	float zx = mViewMatrix._31;
	float zy = mViewMatrix._32;
	float zz = mViewMatrix._33;

	return DirectX::XMFLOAT3(zx, zy, zz);
}

DirectX::XMFLOAT3 Camera::GetRightVector()
{
	float xx = mViewMatrix._11;
	float xy = mViewMatrix._12;
	float xz = mViewMatrix._13;

	return DirectX::XMFLOAT3(xx, xy, xz);
}

DirectX::XMFLOAT3 Camera::GetUpVector()
{
	float yx = mViewMatrix._21;
	float yy = mViewMatrix._22;
	float yz = mViewMatrix._23;

	return DirectX::XMFLOAT3(yx, yy, yz);
}