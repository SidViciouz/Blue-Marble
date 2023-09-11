#pragma once

#include "SceneNode.h"

class PhysicsComponent;

class MeshNode : public SceneNode
{
public:
												MeshNode(string name);
	/*
	* 이 노드를 그린다.
	*/
	virtual void								Draw() override;
	/*
	* 이 노드를 pipeline setting을 하지 않고 그린다.
	*/
	virtual void								DrawWithoutSetting() override;
	/*
	* 이 노드의 상태를 프레임마다 업데이트한다.
	*/
	virtual void								Update() override;
	/*
	* 이 노드가 그리는 mesh의 이름을 반환한다.
	*/
	const string&								GetMeshName() const;
	/*
	* 활성화 여부를 저장한다.
	*/
	void										SetActivated(const bool& value);
	/*
	* 활성화 여부를 반환한다.
	*/
	const bool&									GetActivated() const;
	/*
	* mesh에 입힐 텍스처를 설정한다.
	*/
	void										SetTextureName(const string& name);
	/*
	* physics component를 설정한다.
	*/
	void										SetPhysicsComponent(shared_ptr<PhysicsComponent> physicsComponent);
	/*
	* physics component를 반환한다.
	*/
	shared_ptr<PhysicsComponent>				GetPhysicsComponent() const;

protected:
	/*
	* 그릴 메시의 이름을 저장한다.
	*/
	string										mMeshName;
	/*
	* 활성화 여부를 나타낸다. 즉, 활성화 되어있지 않다면 화면에 그려지지 않는다.
	*/
	bool										mActivated = true;
	/*
	* mesh에 입힐 텍스처의 이름을 나타낸다.
	*/
	string										mTextureName;
	/*
	* physics 기능을 담당하는 컴포넌트이다.
	*/
	shared_ptr<PhysicsComponent>				mPhysicsComponent;
};
