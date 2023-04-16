#pragma once
#include "MeshNode.h"

class UINode : public MeshNode
{
public:
												UINode(string name);
	/*
	* 이 노드를 그린다.
	*/
	virtual void								Draw() override;
	/*
	* 이 노드의 정보들을 매 프레임마다 업데이트이한다.
	*/
	virtual void								Update() override;
	/*
	* 화면에 보여질지 여부를 설정한다.
	*/
	void										SetIsShowUp(bool value);
	/*
	* 화면에 보여질지 여부를 반환한다.
	*/
	const bool&									GetIsShowUp() const;
	/*
	* mIsShowUp을 toggle한다.
	*/
	void										ToggleIsShowUp();

protected:
	/*
	* 화면에 보여질지 여부를 저장하고 있는 변수이다.
	*/
	bool										mIsShowUp = true;

};
