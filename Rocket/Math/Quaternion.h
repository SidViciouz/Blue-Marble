#pragma once
#include "../Util.h"

// x,y,z �� vector part�̰�, w�� scalar part�̴�.

class Quaternion
{
private:
	XMFLOAT4									mQuaternion;
												
public:
												Quaternion();
	Quaternion									operator*(const Quaternion& other) const;

	void										Set(const XMFLOAT4& quaternion);
	void										Set(const float& x,const float& y,const float& z,const float& w);

	/*
	* mQuaternion�� ���� �����ϰ� �״��� quaternion�� �����Ѵ�.
	* ���� mul�� quaternion * mQuaternion���� ����Ѵ�.
	*/
	void										Mul(const XMFLOAT4& quaternion);
	void										Mul(const float& x, const float& y, const float& z, const float& w);

	const XMFLOAT4&								Get();
};
