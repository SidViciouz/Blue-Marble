#pragma once

#include "MeshNode.h"
#include "ItemNode.h"

class InventoryNode : public MeshNode
{
public:
												InventoryNode(string name);
	/*
	* 이 노드와 자식노드를 그린다.
	*/
	virtual void								Draw() override;
	/*
	* 이 노드와 자식노드를 별도의 pipeline setting을 하지 않고 그린다.
	* shadowMap에 그릴 때 사용된다.
	*/
	virtual void								DrawWithoutSetting() override;
	/*
	* 프레임마다 실행되며, 상태를 업데이트한다.
	*/
	virtual void								Update() override;
	/*
	* 화면에 표시될지 여부를 나타내는 변수를 설정한다.
	*/
	void										SetIsShowUp(bool value);
	/*
	* 화면에 표시될지 여부를 반환한다.
	*/
	const bool&									GetIsShowUp() const;
	/*
	* mIsShowUp 변수를 toggle한다. (true->false or false->true)
	*/
	void										ToggleIsShowUp();
	/*
	* item을 저장한다.
	*/
	bool										StoreItem(const int& index);
	/* 
	* item을 drop한다. 
	*/
	bool										DropItem(const int& index);
	/*
	* 물체가 overlap이 된 경우에 inventory 창에서 drop된 아이템이 작게 보여야한다.
	* 이를 설정하는 메서드이다.
	*/
	void										OverlappedNode(shared_ptr<MeshNode> overlapped);

protected:
	/*
	* 화면에 보일 지 여부를 저장한다.
	*/
	bool										mIsShowUp = true;
	/*
	* 각 아이템마다 부여된 무게의 합을 저장한다.
	*/
	int											mWeight = 0;
	/*
	* 저장된 아이템을 작게 출력하기 위한 node이다.
	*/
	array<shared_ptr<ItemNode>, 10>				mStoredItems;
	/*
	* overlap된 아이템을 작게 출력하기 위한 node이다.
	*/
	array<shared_ptr<ItemNode>, 10>				mDroppedItems;
	/*
	* overlapped된 mesh를 가리키는 포인터들이다.
	*/
	array<shared_ptr<MeshNode>, 10>				mOverlappedMeshes;
	/*
	* 저장된 mesh를 가리키는 포인터들이다.
	*/
	array<shared_ptr<MeshNode>, 10>				mStoredMeshes;
};

