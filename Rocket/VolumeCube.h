#pragma once

#include "Volume.h"
#include "TextureResource.h"

class VolumeCube : public Volume
{
public:
												VolumeCube();
	virtual void								Draw() override;

};
