#include "Inventory.h"

Inventory::Inventory(int sceneIndex, const char* fileName, const wchar_t* name)
	: Clickable(sceneIndex, fileName, name)
{

}

void Inventory::Store(string name, shared_ptr<Clickable>&& object)
{
	mInventory[name] = move(object);
}

shared_ptr<Clickable>&& Inventory::Release(string name)
{
	return move(mInventory[name]);
}