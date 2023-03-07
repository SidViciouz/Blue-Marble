#pragma once

#include "Model.h"
#include "TextureResource.h"


class Volume : public Model
{
public:
												Volume();
	void										Load() = delete;
	virtual void								Draw();

	unique_ptr<TextureResource>					mTextureResource;
	
	int											mVolumeIndex = volumeIndex++;
	static int									volumeIndex;
};

using Volumes = unordered_map<string, shared_ptr<Volume>>;