#pragma once

#include "Volume.h"

class VolumeCube : public Volume
{
public:
	VolumeCube();
	virtual void Draw() override;
};
