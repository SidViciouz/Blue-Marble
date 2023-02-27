#pragma once

#include "Entity.h"

class Volume : public Entity
{
public:
	virtual void Intersect(const XMFLOAT3& rayOrigin,const XMFLOAT3& rayDir,float& tMin, float& tMax) const = 0;

	float sc = 0.1f;
	float ac = 0.1f;
};
