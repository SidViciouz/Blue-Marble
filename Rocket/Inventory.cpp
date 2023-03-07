#include "Inventory.h"

Inventory::Inventory(const char* fileName, const wchar_t* name)
	: Clickable(fileName, name)
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