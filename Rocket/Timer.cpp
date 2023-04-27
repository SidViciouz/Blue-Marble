#include "Timer.h"
#include "Util.h"

Timer::Timer()
{
	__int64 frequency;

	//performance frequency�� ���ýÿ� ������.
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

	printf("%f\n", 1.0f / mDeltaTime);

	if (mDeltaTime < 0.0f)
		mDeltaTime = 0.0f;

	mPreviousTime = mCurrentTime;
}

float Timer::GetDeltaTime() const
{
	return mDeltaTime;
}

float Timer::GetTime() const
{
	return mCurrentTime * mSecondPerCount;
}