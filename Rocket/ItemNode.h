#pragma once

#include "MeshNode.h"

class ItemNode : public MeshNode
{
public:
												ItemNode(string mMeshName);

	virtual void								Draw() override;
	virtual void								DrawWithoutSetting() override;
	virtual void								Update() override;
	void										SetMeshName(const string& meshName);
	void										SetDraw(const bool& value);
	const bool&									GetDraw() const;
	void										SetIndex(const int& value);
	const int&									GetIndex() const;
	void										SetIsStored(const bool& value);
	const bool&									GetIsStored() const;

protected:
	bool										mDraw = false;
	int											mIndex = -1;
	bool										mIsStored = false;
};
