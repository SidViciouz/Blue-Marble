#pragma once

#include "Clickable.h"

using Inventories = unordered_map<string, shared_ptr<Clickable>>;

class Inventory : public Clickable
{
public:
												Inventory(int sceneIndex, const char* fileName, const wchar_t* name);
												Inventory(const Inventory& inventory) = delete;
												Inventory(Inventory&& inventory) = delete;
	Inventory&									operator=(const Inventory& inventory) = delete;
	Inventory&									operator=(Inventory&& inventory) = delete;

	void										Store(string name, shared_ptr<Clickable>&& object);
	shared_ptr<Clickable>&&						Release(string name);

	Inventories									mInventory;
};
