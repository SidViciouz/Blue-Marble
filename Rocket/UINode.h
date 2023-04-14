#pragma once
#include "MeshNode.h"

class UINode : public MeshNode
{
public:
												UINode(string name);
	virtual void								Draw() override;
	virtual void								Update() override;
	void										SetIsShowUp(bool value);
	const bool&									GetIsShowUp() const;
	void										ToggleIsShowUp();

protected:

	bool										mIsShowUp = true;

};
