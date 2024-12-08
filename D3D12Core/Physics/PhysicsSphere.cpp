#include "PhysicsSphere.h"

namespace Physics
{
	/// SECTION: Code examples from Real Time Collision Detection by Christer Ericson
	/// These methods were provided by Christer Ericson in his book Real Time Collision Detection,
	/// and adapted to work with DirectX Math.

	bool DoesRayIntersectSphere(DirectX::XMFLOAT3 point, DirectX::XMFLOAT3 direction, Sphere sphere)
	{

		DirectX::XMVECTOR pointV = DirectX::XMLoadFloat3(&point);
		DirectX::XMVECTOR sphereCenterV = DirectX::XMLoadFloat3(&sphere.mCenter);
		DirectX::XMVECTOR directionV = DirectX::XMLoadFloat3(&direction);
		DirectX::XMVECTOR m = DirectX::XMVectorSubtract(pointV, sphereCenterV);

		DirectX::XMVECTOR mDotDirection = DirectX::XMVector3Dot(m, directionV);
		DirectX::XMVECTOR mDotM = DirectX::XMVector3Dot(m, m);

		DirectX::XMFLOAT3 mDotDir3;
		DirectX::XMFLOAT3 mDotM3;
		DirectX::XMStoreFloat3(&mDotDir3, mDotDirection);
		DirectX::XMStoreFloat3(&mDotM3, mDotM);


		float b = mDotDir3.x;
		float c = mDotM3.x - sphere.mRadius * sphere.mRadius;

		if (c > 0.0f && b > 0.0f)
		{
			return false;
		}

		float discr = b * b - c;

		if (discr < 0.0f)
		{
			return false;
		}
		else
		{
			return true;
		}
	}

	/// END SECTION: Code examples from Real Time Collision Detection by Christer Ericson
}