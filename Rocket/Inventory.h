#pragma once

#include "Clickable.h"

class Inventory : public Clickable
{
public:
	Inventory(int sceneIndex, const char* fileName, const wchar_t* name);
	Inventory(const Inventory& inventory) = delete;
	Inventory(Inventory&& inventory) = delete;
	Inventory& operator=(const Inventory& inventory) = delete;
	Inventory& operator=(Inventory&& inventory) = delete;
};