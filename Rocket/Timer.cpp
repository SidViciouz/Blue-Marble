#include "Timer.h"
#include "Util.h"

Timer::Timer()
{
	__int64 frequency;

	//performance frequency는 부팅시에 고정됨.
	QueryPerformanceFrequency((LARGE_INTEGER*)&frequency);

	mSecondPerCount = 1.0f / static_cast<double>(frequency);
}

void Timer::Reset()
{
	QueryPerformanceCounter((LARGE_INTEGER*)&mBaseTime);

	mPreviousTime = mBaseTime;
}

void Timer::Tick()
{
	QueryPerformanceCounter((LARGE_INTEGER*)&mCurrentTime);

	mDeltaTime = (mCurrentTime - mPreviousTime)*mSecondPerCount;

	if (mDeltaTime < 0.0f)
		mDeltaTime = 0.0f;

	mPreviousTime = mCurrentTime;
}