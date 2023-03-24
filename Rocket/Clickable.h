#pragma once

#include "Model.h"


class Clickable : public Model
{
public:
												Clickable(const char* fileName, const wchar_t* name);
												Clickable(const Clickable& clickable) = delete;
												Clickable(Clickable&& clickable) = delete;
	Clickable&									operator=(const Clickable& clickable) = delete;
	Clickable&									operator=(Clickable&& clickable) = delete;

	virtual void								Draw() override;
	static unique_ptr<Clickable>				Spawn(const char* fileName, const wchar_t* name, XMFLOAT3 pos);
};

using Clickables = unordered_map<string, shared_ptr<Clickable>>;