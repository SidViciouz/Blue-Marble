#pragma once

#include "MeshNode.h"

class ItemNode : public MeshNode
{
public:
												ItemNode(string mMeshName);
	/*
	* 인벤토리 창에 물체를 그릴 때 사용된다.
	*/
	virtual void								Draw() override;
	virtual void								DrawWithoutSetting() override;
	virtual void								Update() override;
	/*
	* 이 노드가 그릴 mesh의 이름을 설정한다.
	*/
	void										SetMeshName(const string& meshName);
	/*
	* 이 노드를 그릴 지 여부를 설정한다.
	*/
	void										SetDraw(const bool& value);
	/*
	* 이 노드를 그릴 지 여부를 반환한다.
	*/
	const bool&									GetDraw() const;
	/*
	* 이 노드가 인벤토리 상에서 나타내는 index를 설정한다.
	*/
	void										SetIndex(const int& value);
	/*
	* 이 노드가 인벤토리 상에서 나타내는 index를 반환한다.
	*/
	const int&									GetIndex() const;
	/*
	* 이 노드가 인벤토리에 저장되었는지 여부를 설정한다.
	*/
	void										SetIsStored(const bool& value);
	/*
	* 이 노드가 인벤토리에 저장되었는지 여부를 반환한다.
	*/
	const bool&									GetIsStored() const;

protected:
	/*
	* 이 노드를 그릴지 여부를 나타낸다.
	*/
	bool										mDraw = false;
	/*
	* 이 노드가 인벤토리 상에서 나타내는 index이다.
	*/
	int											mIndex = -1;
	/*
	* 이 노드가 인벤토리에 저장되었는지 여부를 나타낸다.
	*/
	bool										mIsStored = false;
};
