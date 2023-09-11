#pragma once

#include "MeshNode.h"
#include "ItemNode.h"

class InventoryNode : public MeshNode
{
public:
												InventoryNode(string name);
	/*
	* �� ���� �ڽĳ�带 �׸���.
	*/
	virtual void								Draw() override;
	/*
	* �� ���� �ڽĳ�带 ������ pipeline setting�� ���� �ʰ� �׸���.
	* shadowMap�� �׸� �� ���ȴ�.
	*/
	virtual void								DrawWithoutSetting() override;
	/*
	* �����Ӹ��� ����Ǹ�, ���¸� ������Ʈ�Ѵ�.
	*/
	virtual void								Update() override;
	/*
	* ȭ�鿡 ǥ�õ��� ���θ� ��Ÿ���� ������ �����Ѵ�.
	*/
	void										SetIsShowUp(bool value);
	/*
	* ȭ�鿡 ǥ�õ��� ���θ� ��ȯ�Ѵ�.
	*/
	const bool&									GetIsShowUp() const;
	/*
	* mIsShowUp ������ toggle�Ѵ�. (true->false or false->true)
	*/
	void										ToggleIsShowUp();
	/*
	* item�� �����Ѵ�.
	*/
	bool										StoreItem(const int& index);
	/* 
	* item�� drop�Ѵ�. 
	*/
	bool										DropItem(const int& index);
	/*
	* ��ü�� overlap�� �� ��쿡 inventory â���� drop�� �������� �۰� �������Ѵ�.
	* �̸� �����ϴ� �޼����̴�.
	*/
	void										OverlappedNode(shared_ptr<MeshNode> overlapped);

protected:
	/*
	* ȭ�鿡 ���� �� ���θ� �����Ѵ�.
	*/
	bool										mIsShowUp = true;
	/*
	* �� �����۸��� �ο��� ������ ���� �����Ѵ�.
	*/
	int											mWeight = 0;
	/*
	* ����� �������� �۰� ����ϱ� ���� node�̴�.
	*/
	array<shared_ptr<ItemNode>, 10>				mStoredItems;
	/*
	* overlap�� �������� �۰� ����ϱ� ���� node�̴�.
	*/
	array<shared_ptr<ItemNode>, 10>				mDroppedItems;
	/*
	* overlapped�� mesh�� ����Ű�� �����͵��̴�.
	*/
	array<shared_ptr<MeshNode>, 10>				mOverlappedMeshes;
	/*
	* ����� mesh�� ����Ű�� �����͵��̴�.
	*/
	array<shared_ptr<MeshNode>, 10>				mStoredMeshes;
};

