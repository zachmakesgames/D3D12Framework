#pragma once
#include <chrono>

class FrameTimer
{
public:

	inline void Tick()
	{
		if (mFrameCount > 100)
		{
			mAverageFrameTime = mFrameTimeSum / (double)mFrameCount;
			mFrameCount = 0;
			mFrameTimeSum = 0;
		}

		++mFrameCount;
		auto lastTime = mFrameTimer;
		mFrameTimer = std::chrono::high_resolution_clock::now();
		mFrameDuration = mFrameTimer - lastTime;
		mFrameTimeSum += mFrameDuration.count();
	}

	inline double GetFrameTime()
	{
		return mFrameDuration.count();
	}

	inline double GetAverageFrameTime()
	{
		return mAverageFrameTime;
	}

	int mFrameCount = 0;
	double mAverageFrameTime = 0;
	double mFrameTimeSum = 0;

	std::chrono::time_point<std::chrono::high_resolution_clock> mFrameTimer = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> mFrameDuration;
};