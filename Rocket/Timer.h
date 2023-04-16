#pragma once

class Timer
{
public:
												Timer();
	/*
	* �ð��� �ʱ�ȭ�Ѵ�.
	*/
	void										Reset();
	/*
	* �� �����Ӹ��� ȣ��Ǿ� �ð��� ������Ʈ�Ѵ�.
	*/
	void										Tick();
	/*
	* ���� �����Ӱ� �̹� ������ ������ �ð��� ��ȯ�Ѵ�.
	*/
	float										GetDeltaTime() const;
	/*
	* total time�� ��ȯ�Ѵ�.
	*/
	float										GetTime() const;

private:
	__int64										mBaseTime = 0;
	__int64										mPreviousTime = 0;
	__int64										mCurrentTime = 0;
	double										mDeltaTime = 0;
	double										mSecondPerCount = 0.0f;
};