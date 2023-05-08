#pragma once

#include "SceneNode.h"

class PhysicsComponent;

class MeshNode : public SceneNode
{
public:
												MeshNode(string name);
	/*
	* �� ��带 �׸���.
	*/
	virtual void								Draw() override;
	/*
	* �� ��带 pipeline setting�� ���� �ʰ� �׸���.
	*/
	virtual void								DrawWithoutSetting() override;
	/*
	* �� ����� ���¸� �����Ӹ��� ������Ʈ�Ѵ�.
	*/
	virtual void								Update() override;
	/*
	* �� ��尡 �׸��� mesh�� �̸��� ��ȯ�Ѵ�.
	*/
	const string&								GetMeshName() const;
	/*
	* Ȱ��ȭ ���θ� �����Ѵ�.
	*/
	void										SetActivated(const bool& value);
	/*
	* Ȱ��ȭ ���θ� ��ȯ�Ѵ�.
	*/
	const bool&									GetActivated() const;
	/*
	* mesh�� ���� �ؽ�ó�� �����Ѵ�.
	*/
	void										SetTextureName(const string& name);
	/*
	* physics component�� �����Ѵ�.
	*/
	void										SetPhysicsComponent(shared_ptr<PhysicsComponent> physicsComponent);
	/*
	* physics component�� ��ȯ�Ѵ�.
	*/
	shared_ptr<PhysicsComponent>				GetPhysicsComponent() const;

protected:
	/*
	* �׸� �޽��� �̸��� �����Ѵ�.
	*/
	string										mMeshName;
	/*
	* Ȱ��ȭ ���θ� ��Ÿ����. ��, Ȱ��ȭ �Ǿ����� �ʴٸ� ȭ�鿡 �׷����� �ʴ´�.
	*/
	bool										mActivated = true;
	/*
	* mesh�� ���� �ؽ�ó�� �̸��� ��Ÿ����.
	*/
	string										mTextureName;
	/*
	* physics ����� ����ϴ� ������Ʈ�̴�.
	*/
	shared_ptr<PhysicsComponent>				mPhysicsComponent;
};
