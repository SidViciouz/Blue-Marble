#pragma once

#include "Model.h"


class Unclickable : public Model
{
public:
												Unclickable(int sceneIndex, const char* fileName, const wchar_t* name);
												Unclickable(const Unclickable& unclickable) = delete;
												Unclickable(Unclickable&& unclickable) = delete;
	Unclickable&								operator=(const Unclickable& unclickable) = delete;
	Unclickable&								operator=(Unclickable&& unclickable) = delete;
};

using Unclickables = unordered_map<string, shared_ptr<Unclickable>>;