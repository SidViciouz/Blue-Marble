#pragma once

#include "Model.h"
#include "TextureResource.h"

class Volume : public Model
{
public:
	Volume();
	void Load() = delete;
	virtual void Draw();

	unique_ptr<TextureResource> mTextureResource;
};

using Volumes = unordered_map<string, shared_ptr<Volume>>;