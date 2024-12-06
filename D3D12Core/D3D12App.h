#pragma once
#include <queue>
#include <chrono>
#include <unordered_map>
#include "Renderer/D3D12Common.h"
#include "Renderer/Dx12Device.h"
#include "Renderer/Utils.h"
#include "Renderer/Mesh.h"
#include "Renderer/GraphicsPass.h"
#include "Renderer/ResourceGroup.h"
#include "ForwardPass.h"
#include "GBufferPass.h"
#include "LightingPass.h"
#include "GuiPass.h"
#include "UnlitPass.h"
#include "DebugPass.h"
#include "Renderer/AppConstants.h"
#include "InputState.h"
#include "Camera.h"
#include "Scene.h"
#include "FrameTimer.h"
#include "Physics/Triangle.h"

class __declspec(dllexport) D3D12App
{
public:

	D3D12App(HWND window, int swapchainWidth, int swapchainHeight, bool windowed = false);

	void Init();
	void Update();
	void Render();

	void Run();
	void Stop();
	void Resize(UINT newWidth, UINT newHeight);

	void LoadScene(Scene* newScene);

	inline bool IsInited()
	{
		return mInited;
	}

	inline void CleanResources()
	{
		mResourceGroup.mObjects.clear();
		Dx12Device::Destroy();
	}

private:
	void CreateRootSigs();
	void CreateGeometry();
	void CreateShaders();
	void CreatePSOs();

	void PollInputs();

	HWND mWindow;

	UINT mWidth;
	UINT mHeight;

	UINT mFrameCount = 0;

	bool mWindowed;
	bool mRunning = false;

	// A collection of resources used to render the scene. This includes
	//     root signatures, meshes, shaders, textures, psos, and renderable objects
	ResourceGroup mResourceGroup;
	AppConstants mConstants;

	PassGraph mPassGraph;

	std::unordered_map<std::string, GraphicsPass*> mPasses;

	// Great for a simple demo, but ideally each object will handle their own state
	float mObjectRotation = 0;

	std::queue<DirectX::XMINT2> mResizeQueue;

	FrameTimer mFrameTimer;

	InputState mInputState;
	Camera mCamera;

	bool mInited = false;

	bool mShowImGuiDemo = true;

	DirectX::XMFLOAT4 mDebugColor = DirectX::XMFLOAT4(1, 1, 1, 1);
	ImVec4 color;

	Physics::Triangle mTestTriangle = Physics::Triangle(DirectX::XMFLOAT3(0, 0, 0), DirectX::XMFLOAT3(5, 0, 0), DirectX::XMFLOAT3(0, 5, 0));
	bool mRayIntersectsTriangle = false;
	DirectX::XMFLOAT3 mHitPos = DirectX::XMFLOAT3(0, 0, 0);
};

