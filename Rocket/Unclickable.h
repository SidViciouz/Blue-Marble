#pragma once

#include "Model.h"

class Unclickable : public Model
{
	Unclickable(int sceneIndex, const char* fileName, const wchar_t* name);
	Unclickable(const Unclickable& unclickable) = delete;
	Unclickable(Unclickable&& unclickable) = delete;
	Unclickable& operator=(const Unclickable& unclickable) = delete;
	Unclickable& operator=(Unclickable&& unclickable) = delete;
};
