#pragma once

#include "Scene.h"
#include "MeshNode.h"
#include "CameraNode.h"
#include "UINode.h"
#include "TextNode.h"
#include "LightNode.h"
#include "Engine.h"

/*
* 프로그램을 시작할 때 나오는 화면에 해당하는 scene이다.
*/
class MenuScene : public Scene
{
public:
												MenuScene();
	/*
	* 이 scene을 초기화한다.
	*/
	void										Initialize();
	/*
	* 이 scene의 각종 오브젝트들을 초기화한다.
	*/
	virtual void								Update(const Timer& timer) override;
};
