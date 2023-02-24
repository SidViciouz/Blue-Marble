#pragma once

#include "Model.h"

class Clickable : public Model
{
	Clickable(int sceneIndex, const char* fileName, const wchar_t* name);
	Clickable(const Clickable& clickable) = delete;
	Clickable(Clickable&& clickable) = delete;
	Clickable& operator=(const Clickable& clickable) = delete;
	Clickable& operator=(Clickable&& clickable) = delete;
};