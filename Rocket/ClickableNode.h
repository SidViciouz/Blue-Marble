#pragma once
#include "MeshNode.h"

class ClickableNode : public MeshNode
{
public:
												ClickableNode(string name);
	virtual void								Draw() override;
	virtual void								Update() override;
	void										SetIsShowUp(bool value);
	const bool&									GetIsShowUp() const;
	void										ToggleIsShowUp();

protected:

	bool										mIsShowUp = true;
};
