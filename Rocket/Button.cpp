#include "Button.h"

Button::Button(int sceneIndex, const char* fileName, const wchar_t* name)
	: Clickable(sceneIndex, fileName, name)
{

}

void Button::Click()
{
	lambda();
}

void Button::Set(function<void(void)> exp)
{
	lambda = exp;
}