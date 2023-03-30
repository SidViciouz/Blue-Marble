#pragma once
#include "../Util.h"

// x,y,z 는 vector part이고, w는 scalar part이다.

class Quaternion
{
private:
	XMFLOAT4									mQuaternion;
												
public:
												Quaternion();
												Quaternion(const float& x, const float& y, const float& z, const float& w);
	Quaternion									operator*(const Quaternion& other) const;

	void										Set(const XMFLOAT4& quaternion);
	void										Set(const Quaternion& quaternion);
	void										Set(const float& x,const float& y,const float& z,const float& w);

	/*
	* mQuaternion을 먼저 적용하고 그다음 quaternion을 적용한다.
	* 따라서 mul은 quaternion * mQuaternion으로 계산한다.
	*/
	void										Mul(const XMFLOAT4& quaternion);
	void										Mul(const Quaternion& quaternion);
	void										Mul(const float& x, const float& y, const float& z, const float& w);

	const XMFLOAT4&								Get();
};
