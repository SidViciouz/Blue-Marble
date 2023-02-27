#pragma once

#include "Volume.h"

class VolumeSphere : public Volume
{
public:
	virtual void Intersect(const XMFLOAT3& rayOrigin, const XMFLOAT3& rayDir, float& tMin, float& tMax) const override;

	float mRadius = 5.0f;
};
