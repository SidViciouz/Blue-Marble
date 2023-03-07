#pragma once

class Timer
{
public:
												Timer();
	void										Reset();
	void										Tick();
	float										GetDeltaTime() const;

private:
	__int64										mBaseTime = 0;
	__int64										mPreviousTime = 0;
	__int64										mCurrentTime = 0;
	double										mDeltaTime = 0;
	double										mSecondPerCount = 0.0f;
};