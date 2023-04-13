#pragma once

#include "MeshNode.h"

class IconNode : public MeshNode
{
public:
												IconNode(string mMeshName);

	virtual void								Draw() override;
	virtual void								DrawWithoutSetting() override;
	virtual void								Update() override;
	void										SetMeshName(const string& meshName);
	void										SetDraw(const bool& value);
	const bool&									GetDraw() const;

protected:
	bool										mDraw = false;
};
