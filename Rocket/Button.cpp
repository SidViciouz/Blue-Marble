#include "Button.h"

Button::Button(const char* fileName, const wchar_t* name)
	: Clickable(fileName, name)
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