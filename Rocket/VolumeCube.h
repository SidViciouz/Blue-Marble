#pragma once

#include "Volume.h"

class VolumeCube : public Volume
{
public:
	XMFLOAT3 mVolumeScale = { 5.0f,5.0f,5.0f };
};
