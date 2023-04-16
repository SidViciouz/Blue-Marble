#pragma once

#include "InventoryNode.h"
#include "TextNode.h"
#include "Engine.h"
#include "ItemNode.h"

/*
* 게임의 메인 scene이다.
*/
class MainScene : public Scene
{
public:
												MainScene();
	/*
	* 이 scene의 각종 오브젝트들의 상태를 초기화한다.
	*/
	void										Initialize();
	/*
	* scene의 모드 오브젝트들을 프레임마다 업데이트한다.
	*/
	virtual void								UpdateScene(const Timer& timer) override;

	
	/*
	* 이 scene내의 몇몇 노드들을 임시로 가리키고 있는 포인터이다.
	* 수정 예정이다.
	*/
	shared_ptr<MeshNode>						boxMesh;
	shared_ptr<MeshNode>						ballMesh;
	shared_ptr<MeshNode>						groundMesh;
	shared_ptr<InventoryNode>					inventory;
};
