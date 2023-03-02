#pragma once

#include "Volume.h"

class VolumeSphere : public Volume
{
public:
	float mRadius = 5.0f;
	virtual void Draw() override;
};
