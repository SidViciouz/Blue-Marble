#pragma once

#include "Util.h"
#include "Math/Position.h"
#include "Math/Quaternion.h"
#include "Constant.h"
#include "CollisionComponent.h"
#include "RigidBodyComponent.h"
#include "InputComponentBase.h"

class SceneNode
{
public:
												SceneNode();
	/*
	* �� ��带 ���� �׸��� �ڽ� ������ �׸���.
	*/
	virtual void								Draw();
	/*
	* pipeline setting�� ���� �ʰ� �� ���� �ڽ� ������ �׸���.
	*/
	virtual void								DrawWithoutSetting();
	/*
	* �����Ӹ��� �����͸� ������Ʈ�Ѵ�.
	*/
	virtual void								Update();
	/*
	* �� ����� collisionComponent�� ��� ����� collisionComponent�� �浹�ϴ����� ���ο� �浹������ ��ȯ�Ѵ�.
	*/
	virtual bool								IsColliding(SceneNode* counterPart, CollisionInfo& collisionInfo);
	/*
	* �� ��忡 �ڽ� ��带 �߰��Ѵ�.
	*/
	void										AddChild(shared_ptr<SceneNode> child);
	/*
	* �ڽ� ��带 �����Ѵ�.
	*/
	void										RemoveChild(shared_ptr<SceneNode> child);
	/*
	* �θ� ������ ������� ��ġ�� �����Ѵ�.
	*/
	void										SetRelativePosition(const XMFLOAT3& position);
	void										SetRelativePosition(const float& x,const float& y, const float& z);
	/*
	* �θ� ������ ������� quaternion�� �����Ѵ�.
	*/
	void										SetRelativeQuaternion(const XMFLOAT4& quaternion);
	void										SetRelativeQuaternion(const float& x,const float& y,const float& z,const float& w);
	/*
	* scale�� �����Ѵ�.
	*/
	void										SetScale(const XMFLOAT3& scale);
	void										SetScale(const float& x,const float& y, const float& z);
	/*
	* �� ����� global position�� �����Ѵ�.
	*/
	void										SetAccumulatedPosition(const XMFLOAT3& position);
	void										SetAccumulatedPosition(const float& x, const float& y, const float& z);
	/*
	* ������� ��ġ�� �Է����� ���� ũ�⸸ŭ ���Ѵ�.
	*/
	void										AddRelativePosition(const XMFLOAT3& position);
	void										AddRelativePosition(const float& x, const float& y, const float& z);
	/*
	* ������� quaternion�� �Է����� ���� quaternion�� ���Ѵ�.
	*/
	void										MulRelativeQuaternion(const Quaternion& quaternion);
	void										MulRelativeQuaternion(const XMFLOAT4& quaternion);
	void										MulRelativeQuaternion(const float& x, const float& y, const float& z, const float& w);

	void										AddAccumulatedPosition(const XMFLOAT3& position);
	void										AddAccumulatedPosition(const float& x, const float& y, const float& z);
	void										MulAccumulatedQuaternion(const Quaternion& quaternion);
	void										MulAccumulatedQuaternion(const XMFLOAT4& quaternion);
	void										MulAccumulatedQuaternion(const float& x, const float& y, const float& z, const float& w);
	/*
	* ������� ��ġ�� d*position�� ���Ѵ�.
	*/
	void										MulAddRelativePosition(const float& d,const XMFLOAT3& position);
	/*
	* ������� ��ġ�� ��ȯ�Ѵ�.
	*/
	Position									GetRelativePosition();
	/*
	* ������� quaternion�� ��ȯ�Ѵ�.
	*/
	Quaternion									GetRelativeQuaternion();
	/*
	* scale�� ��ȯ�Ѵ�.
	*/
	XMFLOAT3									GetScale();
	/*
	* global position�� ��ȯ�Ѵ�.
	*/
	Position									GetAccumulatedPosition();
	/*
	* global quaternion�� ��ȯ�Ѵ�.
	*/
	Quaternion									GetAccumulatedQuaternion();
	/*
	* �� ��忡 ������ collision component�� ����Ų��.
	*/
	shared_ptr<CollisionComponent>				mCollisionComponent;
	/*
	* �� ��忡 ������ rigid body component�� ����Ų��.
	*/
	shared_ptr<RigidBodyComponent>				mRigidBodyComponent;
	/*
	* �� ��忡 ������ input component�� ����Ų��.
	*/
	shared_ptr<InputComponentBase>				mInputComponent;
	/*
	* �� ��尡 �����Ӹ��� ���ε��� �����͸� ��Ÿ����.
	*/
	obj											mObjFeature;
	/*
	* �ڽ� ������ ����Ų��.
	*/
	vector<shared_ptr<SceneNode>>				mChildNodes;
	/*
	* �θ� ��带 ����Ų��.
	*/
	SceneNode*									mParentNode = nullptr;
	/*
	* node table�������� index�� ����Ų��.
	*/
	int											mSceneNodeIndex;
	/*
	* ��� node���� ����ִ� table�̴�.
	*/
	static bool									mSceneNodeNumTable[MAX_SCENE_NODE_NUM];

protected:
	/*
	* �θ� ������ ������� position�̴�.
	*/
	Position									mRelativePosition;
	/*
	* �θ� ������ ������� quaternion�̴�.
	*/
	Quaternion									mRelativeQuaternion;
	/*
	* scale�� ��Ÿ����.
	*/
	XMFLOAT3									mScale;
	/*
	* global position�� ��Ÿ����.
	*/
	Position									mAccumulatedPosition;
	/*
	* global quaternion�� ��Ÿ����.
	*/
	Quaternion									mAccumulatedQuaternion;
};