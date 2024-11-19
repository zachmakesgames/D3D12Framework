#include "Camera.h"

Camera::Camera() :
	mPosition({ 0, 0, 0 }), mYaw(0), mPitch(0)
{
	UpdateMatrix();
}

DirectX::XMFLOAT4X4 Camera::GetViewMatrix()
{
	return mViewMatrix;
}

void Camera::SetPosition(DirectX::XMFLOAT3 newPosition)
{
	mPosition = newPosition;
	UpdateMatrix();
}

void Camera::SetPosition(DirectX::XMVECTOR newPositionV)
{
	DirectX::XMStoreFloat3(&mPosition, newPositionV);
	UpdateMatrix();
}

void Camera::AddPosition(DirectX::XMFLOAT3 position)
{
	DirectX::XMVECTOR currentPosition = DirectX::XMLoadFloat3(&mPosition);
	DirectX::XMVECTOR positionOffset = DirectX::XMLoadFloat3(&position);
	DirectX::XMVECTOR newPos = DirectX::XMVectorAdd(currentPosition, positionOffset);
	DirectX::XMStoreFloat3(&mPosition, newPos);
	UpdateMatrix();
}

void Camera::AddPosition(DirectX::XMVECTOR positionv)
{
	DirectX::XMVECTOR currentPosition = DirectX::XMLoadFloat3(&mPosition);
	DirectX::XMVECTOR newPos = DirectX::XMVectorAdd(currentPosition, positionv);
	DirectX::XMStoreFloat3(&mPosition, newPos);
	UpdateMatrix();
}

void Camera::SetYaw(float newYaw)
{
	mYaw = newYaw;
	UpdateMatrix();
}

void Camera::SetPitch(float newPitch)
{
	mPitch = newPitch;
	UpdateMatrix();
}

void Camera::AddYaw(float yaw)
{
	mYaw += yaw;
	UpdateMatrix();
}

void Camera::AddPitch(float pitch)
{	
	mPitch += pitch;
	UpdateMatrix();
}

DirectX::XMFLOAT3 Camera::GetForwardVector()
{
	float zx = mViewMatrix._13;
	float zy = mViewMatrix._23;
	float zz = mViewMatrix._33;

	return DirectX::XMFLOAT3(zx, zy, zz);
}

DirectX::XMFLOAT3 Camera::GetRightVector()
{
	float xx = mViewMatrix._11;
	float xy = mViewMatrix._21;
	float xz = mViewMatrix._31;

	return DirectX::XMFLOAT3(xx, xy, xz);
}

DirectX::XMFLOAT3 Camera::GetUpVector()
{
	float yx = mViewMatrix._12;
	float yy = mViewMatrix._22;
	float yz = mViewMatrix._32;

	return DirectX::XMFLOAT3(yx, yy, yz);
}

void Camera::UpdateMatrix()
{
	DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(mPosition.x, mPosition.y, mPosition.z);
	DirectX::XMMATRIX rotationYaw = DirectX::XMMatrixRotationY(mYaw);
	DirectX::XMMATRIX rotationPitch = DirectX::XMMatrixRotationX(mPitch);

	DirectX::XMMATRIX rotation = DirectX::XMMatrixMultiply(rotationYaw, rotationPitch);

	DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixMultiply(translation, rotation);

	DirectX::XMStoreFloat4x4(&mViewMatrix, viewMatrix);
}

DirectX::XMFLOAT3 Camera::GetCameraPosition()
{
	// Need to invert the position to get a true world
	// representation for the camera
	DirectX::XMVECTOR posV = DirectX::XMLoadFloat3(&mPosition);
	posV = DirectX::XMVectorScale(posV, -1.f);
	DirectX::XMFLOAT3 pos3;
	DirectX::XMStoreFloat3(&pos3, posV);
	return pos3;
}