#pragma once

#include "Model.h"

class Volume : public Model
{
public:
	Volume();
	void Load() = delete;

	float sc = 0.1f;
	float ac = 0.1f;
};

using Volumes = unordered_map<string, shared_ptr<Volume>>;