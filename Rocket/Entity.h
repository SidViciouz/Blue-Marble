#pragma once

#include "Util.h"
#include "Math/Quaternion.h"
#include "Math/Position.h"
#include "Constant.h"

class Entity
{
protected:
	Position									mPosition;
	Quaternion									mQuaternion;
												
public:
	bool										mDirty = true;
	virtual void								Update();

	void										SetPosition(const XMFLOAT3& position);
	void										SetPosition(const float& x, const float& y, const float& z);
	const XMFLOAT3&								GetPosition();

	void										AddPosition(const XMFLOAT3& d);
	void										AddPosition(const float& x,const float& y,const float& z);

	void										SetQuaternion(const XMFLOAT4& quaternion);
	void										SetQuaternion(const float& x, const float& y, const float& z, const float& w);
	const XMFLOAT4&								GetQuaternion();

	obj											mObjFeature =
	{ {1.0f,0.0f,0.0f,0.0f,
		0.0f,1.0f,0.0f,0.0f,
		0.0f,0.0f,1.0f,0.0f,
		0.0f,0.0f,0.0f,1.0f},
		{0.9f,0.9f,0.9f},
		0.5f,
		{0.2f,0.2f,0.1f}
	};

};
