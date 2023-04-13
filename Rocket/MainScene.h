#pragma once

#include "InventoryNode.h"
#include "TextNode.h"
#include "Engine.h"
#include "ItemNode.h"

class MainScene : public Scene
{
public:
												MainScene();
	void										Initialize();
	virtual void								UpdateScene(const Timer& timer) override;

	shared_ptr<MeshNode>						boxMesh;
	shared_ptr<MeshNode>						ballMesh;
	shared_ptr<MeshNode>						groundMesh;
	shared_ptr<InventoryNode>					inventory;
};
