#pragma once

#include "D3D12Common.h"
#include "RenderObject.h"

class GraphicsPass
{
public:
	GraphicsPass* mPreviousPass = nullptr;
	GraphicsPass* mNextPass = nullptr;
	std::string mPassName = "";

	std::vector<RenderObject*> mRenderObjectRefs;

	virtual void PreRender() {};
	virtual void Render() {};
	virtual void PostRender() {};

	inline GraphicsPass(std::string passName) : mPassName(passName) {};
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

	inline std::string Walk()
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
			currentPass->PreRender();
			currentPass->Render();
			currentPass->PostRender();
			
			while (currentPass->mNextPass != nullptr)
			{
				currentPass = currentPass->mNextPass;
				graph_str += "->" + currentPass->mPassName;
			}
		}

		return graph_str;
	}


};
