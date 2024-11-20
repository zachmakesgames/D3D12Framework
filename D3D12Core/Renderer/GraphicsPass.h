#pragma once

#include "D3D12Common.h"
#include "RenderObject.h"
#include "ResourceGroup.h"
#include "AppConstants.h"

class GraphicsPass
{
public:
	GraphicsPass* mPreviousPass = nullptr;
	GraphicsPass* mNextPass = nullptr;
	std::string mPassName = "";

	std::vector<RenderObject*> mRenderObjectRefs;
	ResourceGroup* mResourceGroup;
	AppConstants* mConstants;

	inline GraphicsPass(ResourceGroup* resourceGroup, AppConstants* constants) 
		: mResourceGroup(resourceGroup), mConstants(constants)
	{}

	virtual void PreRender(UINT frameNumber) {};
	virtual void Render(UINT frameNumber) {};
	virtual void PostRender(UINT frameNumber) {};

	inline GraphicsPass(std::string passName) : mPassName(passName), mResourceGroup(nullptr), mConstants(nullptr) {};

	inline void RegisterRenderObject(RenderObject* objRef)
	{
		auto ref = std::find(mRenderObjectRefs.begin(), mRenderObjectRefs.end(), objRef);
		if (ref == mRenderObjectRefs.end())
		{
			mRenderObjectRefs.push_back(objRef);
		}
	}

	inline void RemoveRenderObject(RenderObject* objRef)
	{
		auto refInVector = std::find(mRenderObjectRefs.begin(), mRenderObjectRefs.end(), objRef);
		if (refInVector != mRenderObjectRefs.end())
		{
			mRenderObjectRefs.erase(refInVector);
		}
	}
};

class PassGraph
{
public:
	GraphicsPass* mFirstPass = nullptr;
	GraphicsPass* mLastPass = nullptr;

	inline void AddPass(GraphicsPass* newPass)
	{
		GraphicsPass* currentNewPass = newPass;

		while (currentNewPass->mNextPass != nullptr)
		{
			currentNewPass = currentNewPass->mNextPass;
		}


		if (mLastPass == nullptr)
		{
			if (mFirstPass == nullptr)
			{
				mFirstPass = newPass;
				mLastPass = currentNewPass;
			}
			else
			{
				GraphicsPass* currentPass = mFirstPass;
				while (currentPass->mNextPass != nullptr)
				{
					currentPass = currentPass->mNextPass;
				}
				currentPass->mNextPass = newPass;
				newPass->mPreviousPass = currentPass;
				mLastPass = currentNewPass;
			}
		}
		else
		{
			mLastPass->mNextPass = newPass;
			newPass->mPreviousPass = mLastPass;
			mLastPass = currentNewPass;
		}
	}

	inline std::string Walk(UINT frameNum)
	{
		std::string graph_str = "Pass graph:";
		if (mFirstPass == nullptr)
		{
			graph_str += " No passes in this graph";
		}
		else
		{
			GraphicsPass* currentPass = mFirstPass;
			graph_str += " " + currentPass->mPassName;
			currentPass->PreRender(frameNum);
			currentPass->Render(frameNum);
			currentPass->PostRender(frameNum);
			
			while (currentPass->mNextPass != nullptr)
			{
				currentPass = currentPass->mNextPass;
				graph_str += "->" + currentPass->mPassName;
				currentPass->PreRender(frameNum);
				currentPass->Render(frameNum);
				currentPass->PostRender(frameNum);
			}
		}

		return graph_str;
	}
};
