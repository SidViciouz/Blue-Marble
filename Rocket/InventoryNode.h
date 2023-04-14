#pragma once

#include "Util.h"
#include "MeshNode.h"
#include "ItemNode.h"

class InventoryNode : public MeshNode
{
public:
												InventoryNode(string name);
	virtual void								Draw() override;
	virtual void								DrawWithoutSetting() override;
	virtual void								Update() override;
	void										SetIsShowUp(bool value);
	const bool& GetIsShowUp() const;
	void										ToggleIsShowUp();
	bool										StoreItem(const int& index);
	bool										DropItem(const int& index);
	void										OverlappedNode(shared_ptr<MeshNode> overlapped);
	void										UnOverlapped();

protected:

	bool										mIsShowUp = true;
	int											mWeight = 0;

	array<shared_ptr<ItemNode>, 10>				mStoredItems;
	array<shared_ptr<ItemNode>, 10>				mDroppedItems;
	array<shared_ptr<MeshNode>, 10>				mOverlappedMeshes;
	array<shared_ptr<MeshNode>, 10>				mStoredMeshes;
};

