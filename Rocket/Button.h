#pragma once

#include "Clickable.h"
#include <functional>

class Button : public Clickable
{
public:
	Button(int sceneIndex, const char* fileName, const wchar_t* name);
	Button(const Button& button) = delete;
	Button(Button&& button) = delete;
	Button& operator=(const Button& button) = delete;
	Button& operator=(Button&& button) = delete;

	void Click();
	
	void Set(function<void(void)> exp);

	function<void(void)> lambda;
};