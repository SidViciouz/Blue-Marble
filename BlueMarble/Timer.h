#pragma once

class Timer
{
public:
												Timer();
	/*
	* 시간을 초기화한다.
	*/
	void										Reset();
	/*
	* 매 프레임마다 호출되어 시간을 업데이트한다.
	*/
	void										Tick();
	/*
	* 지난 프레임과 이번 프레임 사이의 시간을 반환한다.
	*/
	float										GetDeltaTime() const;
	/*
	* total time을 반환한다.
	*/
	float										GetTime() const;

private:
	__int64										mBaseTime = 0;
	__int64										mPreviousTime = 0;
	__int64										mCurrentTime = 0;
	double										mDeltaTime = 0;
	double										mSecondPerCount = 0.0f;
};