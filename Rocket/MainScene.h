#pragma once

#include "Scene.h"
#include "MeshNode.h"
#include "VolumeNode.h"

class MainScene : public Scene
{
public:
												MainScene();
	void										Initialize();
	virtual void								UpdateScene(float deltaTime) override;

	shared_ptr<MeshNode>						boxMesh;
	shared_ptr<MeshNode>						ballMesh;
	shared_ptr<MeshNode>						groundMesh;
};
