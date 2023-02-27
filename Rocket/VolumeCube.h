#pragma once

#include "Volume.h"

class VolumeCube : public Volume
{
public:
	virtual void Intersect(const XMFLOAT3& rayOrigin, const XMFLOAT3& rayDir, float& tMin, float& tMax) const override;

	XMFLOAT3 mScale = { 5.0f,5.0f,5.0f };
};
