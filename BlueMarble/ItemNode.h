#pragma once

#include "MeshNode.h"

class ItemNode : public MeshNode
{
public:
												ItemNode(string mMeshName);
	/*
	* �κ��丮 â�� ��ü�� �׸� �� ���ȴ�.
	*/
	virtual void								Draw() override;
	virtual void								DrawWithoutSetting() override;
	virtual void								Update() override;
	/*
	* �� ��尡 �׸� mesh�� �̸��� �����Ѵ�.
	*/
	void										SetMeshName(const string& meshName);
	/*
	* �� ��带 �׸� �� ���θ� �����Ѵ�.
	*/
	void										SetDraw(const bool& value);
	/*
	* �� ��带 �׸� �� ���θ� ��ȯ�Ѵ�.
	*/
	const bool&									GetDraw() const;
	/*
	* �� ��尡 �κ��丮 �󿡼� ��Ÿ���� index�� �����Ѵ�.
	*/
	void										SetIndex(const int& value);
	/*
	* �� ��尡 �κ��丮 �󿡼� ��Ÿ���� index�� ��ȯ�Ѵ�.
	*/
	const int&									GetIndex() const;
	/*
	* �� ��尡 �κ��丮�� ����Ǿ����� ���θ� �����Ѵ�.
	*/
	void										SetIsStored(const bool& value);
	/*
	* �� ��尡 �κ��丮�� ����Ǿ����� ���θ� ��ȯ�Ѵ�.
	*/
	const bool&									GetIsStored() const;

protected:
	/*
	* �� ��带 �׸��� ���θ� ��Ÿ����.
	*/
	bool										mDraw = false;
	/*
	* �� ��尡 �κ��丮 �󿡼� ��Ÿ���� index�̴�.
	*/
	int											mIndex = -1;
	/*
	* �� ��尡 �κ��丮�� ����Ǿ����� ���θ� ��Ÿ����.
	*/
	bool										mIsStored = false;
};
