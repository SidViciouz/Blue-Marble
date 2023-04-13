#pragma once

#include "ClickableNode.h"
#include "TextNode.h"
#include "Engine.h"
#include "IconNode.h"

class MainScene : public Scene
{
public:
												MainScene();
	void										Initialize();
	virtual void								UpdateScene(const Timer& timer) override;

	shared_ptr<MeshNode>						boxMesh;
	shared_ptr<MeshNode>						ballMesh;
	shared_ptr<MeshNode>						groundMesh;
	shared_ptr<ClickableNode>					inventory;
	shared_ptr<IconNode>						icon1;
};
