#pragma once

#include "Util.h"
#include "MeshNode.h"
#include "ItemNode.h"

class InventoryNode : public MeshNode
{
public:
												InventoryNode(string name);
	virtual void								Draw() override;
	virtual void								Update() override;
	void										SetIsShowUp(bool value);
	const bool& GetIsShowUp() const;
	void										ToggleIsShowUp();
	void										SetClickedMesh(const int& index);
	void										UnsetHaveClickedMesh();
	const int&									GetClickedMeshIndex() const;
	const bool&									HaveClickedMesh() const;
	bool										StoreItem(const int& index);
	bool										DropItem(const int& index);

protected:

	bool										mIsShowUp = true;
	int											mWeight = 0;
	bool										mHaveClickedMesh = false;
	int											mClickedMeshIndex;
	array<shared_ptr<ItemNode>, 10>				mStoredItems;
	array<shared_ptr<ItemNode>, 10>				mDroppedItems;
};

