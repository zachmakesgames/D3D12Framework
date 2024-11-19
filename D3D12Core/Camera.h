#pragma once
#include <DirectXMath.h>
#include <string>
#include <Windows.h>

class __declspec(dllexport) Camera
{
public:
	Camera();

	DirectX::XMFLOAT4X4 GetViewMatrix();
	void SetPosition(DirectX::XMFLOAT3 newPosition);
	void SetPosition(DirectX::XMVECTOR newPositionV);
	void AddPosition(DirectX::XMFLOAT3 position);
	void AddPosition(DirectX::XMVECTOR positionv);

	void SetYaw(float newYaw);
	void SetPitch(float newPitch);

	void AddYaw(float yaw);
	void AddPitch(float pitch);

	DirectX::XMFLOAT3 GetForwardVector();
	DirectX::XMFLOAT3 GetRightVector();
	DirectX::XMFLOAT3 GetUpVector();
	DirectX::XMFLOAT3 GetCameraPosition();

private:
	void UpdateMatrix();
	DirectX::XMFLOAT4X4 mViewMatrix;
	DirectX::XMFLOAT3 mPosition;
	float mYaw;
	float mPitch;
};

