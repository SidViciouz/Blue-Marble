#pragma once

#include "InventoryNode.h"
#include "TextNode.h"
#include "Engine.h"
#include "ItemNode.h"
#include "WorldNode.h"


/*
* ������ ���� scene�̴�.
*/
class MainScene : public Scene
{
public:
												MainScene();
	/*
	* �� scene�� ���� ������Ʈ���� ���¸� �ʱ�ȭ�Ѵ�.
	*/
	void										Initialize();
	/*
	* scene�� ��� ������Ʈ���� �����Ӹ��� ������Ʈ�Ѵ�.
	*/
	virtual void								UpdateScene(const Timer& timer) override;

	virtual void								DrawScene() const override;
	
	/*
	* �� scene���� ��� ������ �ӽ÷� ����Ű�� �ִ� �������̴�.
	* ���� �����̴�.
	*/
	shared_ptr<MeshNode>						boxMesh;
	shared_ptr<MeshNode>						ballMesh;
	shared_ptr<MeshNode>						groundMesh;
	shared_ptr<InventoryNode>					inventory;
	shared_ptr<WorldNode>						worldMesh;

};
