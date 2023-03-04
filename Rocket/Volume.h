#pragma once

#include "Model.h"

class Volume : public Model
{
public:
	Volume();
	void Load() = delete;
	virtual void Draw();
};

using Volumes = unordered_map<string, shared_ptr<Volume>>;