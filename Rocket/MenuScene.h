#pragma once

#include "Scene.h"
#include "VolumeNode.h"
#include "MeshNode.h"
#include "CameraNode.h"
#include "UINode.h"
#include "TextNode.h"
#include "LightNode.h"
#include "Engine.h"

/*
* ���α׷��� ������ �� ������ ȭ�鿡 �ش��ϴ� scene�̴�.
*/
class MenuScene : public Scene
{
public:
												MenuScene();
	/*
	* �� scene�� �ʱ�ȭ�Ѵ�.
	*/
	void										Initialize();
	/*
	* �� scene�� ���� ������Ʈ���� �ʱ�ȭ�Ѵ�.
	*/
	virtual void								UpdateScene(const Timer& timer) override;
};
