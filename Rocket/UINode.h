#pragma once
#include "MeshNode.h"

class UINode : public MeshNode
{
public:
												UINode(string name);
	/*
	* �� ��带 �׸���.
	*/
	virtual void								Draw() override;
	/*
	* �� ����� �������� �� �����Ӹ��� ������Ʈ���Ѵ�.
	*/
	virtual void								Update() override;
	/*
	* ȭ�鿡 �������� ���θ� �����Ѵ�.
	*/
	void										SetIsShowUp(bool value);
	/*
	* ȭ�鿡 �������� ���θ� ��ȯ�Ѵ�.
	*/
	const bool&									GetIsShowUp() const;
	/*
	* mIsShowUp�� toggle�Ѵ�.
	*/
	void										ToggleIsShowUp();

protected:
	/*
	* ȭ�鿡 �������� ���θ� �����ϰ� �ִ� �����̴�.
	*/
	bool										mIsShowUp = true;

};
