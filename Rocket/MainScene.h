#pragma once

#include "Scene.h"
#include "MeshNode.h"
#include "VolumeNode.h"
#include "CameraNode.h"
#include "ClickableNode.h"

class MainScene : public Scene
{
public:
												MainScene();
	void										Initialize();
	virtual void								UpdateScene(const Timer& timer) override;

	shared_ptr<MeshNode>						boxMesh;
	shared_ptr<MeshNode>						ballMesh;
	shared_ptr<MeshNode>						groundMesh;
};
