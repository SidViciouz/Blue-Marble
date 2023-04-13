#pragma once

#include "Scene.h"
#include "VolumeNode.h"
#include "MeshNode.h"
#include "CameraNode.h"
#include "ClickableNode.h"
#include "TextNode.h"
#include "LightNode.h"
#include "Engine.h"

class MenuScene : public Scene
{
public:
												MenuScene();
	void										Initialize();
	virtual void								UpdateScene(const Timer& timer) override;
};
