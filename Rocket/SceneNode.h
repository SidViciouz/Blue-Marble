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

	virtual void								Draw();
	virtual void								DrawWithoutSetting();
	virtual void								Update();
	virtual bool								IsColliding(SceneNode* counterPart, CollisionInfo& collisionInfo);

	void										AddChild(shared_ptr<SceneNode> child);
	void										RemoveChild(shared_ptr<SceneNode> child);

	void										SetRelativePosition(const XMFLOAT3& position);
	void										SetRelativePosition(const float& x,const float& y, const float& z);
	void										SetRelativeQuaternion(const XMFLOAT4& quaternion);
	void										SetRelativeQuaternion(const float& x,const float& y,const float& z,const float& w);
	void										SetScale(const XMFLOAT3& scale);
	void										SetScale(const float& x,const float& y, const float& z);
	void										SetAccumulatedPosition(const XMFLOAT3& position);
	void										SetAccumulatedPosition(const float& x, const float& y, const float& z);

	void										AddRelativePosition(const XMFLOAT3& position);
	void										AddRelativePosition(const float& x, const float& y, const float& z);
	void										MulRelativeQuaternion(const Quaternion& quaternion);
	void										MulRelativeQuaternion(const XMFLOAT4& quaternion);
	void										MulRelativeQuaternion(const float& x, const float& y, const float& z, const float& w);

	void										AddAccumulatedPosition(const XMFLOAT3& position);
	void										AddAccumulatedPosition(const float& x, const float& y, const float& z);
	void										MulAccumulatedQuaternion(const Quaternion& quaternion);
	void										MulAccumulatedQuaternion(const XMFLOAT4& quaternion);
	void										MulAccumulatedQuaternion(const float& x, const float& y, const float& z, const float& w);

	void										MulAddRelativePosition(const float& d,const XMFLOAT3& position);

	Position									GetRelativePosition();
	Quaternion									GetRelativeQuaternion();
	XMFLOAT3									GetScale();
	Position									GetAccumulatedPosition();
	Quaternion									GetAccumulatedQuaternion();

	shared_ptr<CollisionComponent>				mCollisionComponent;
	shared_ptr<RigidBodyComponent>				mRigidBodyComponent;
	shared_ptr<InputComponentBase>				mInputComponent;

	obj											mObjFeature;

	vector<shared_ptr<SceneNode>>				mChildNodes;
	SceneNode*									mParentNode = nullptr;


	int											mSceneNodeIndex;

	static bool									mSceneNodeNumTable[MAX_SCENE_NODE_NUM];

protected:

	Position									mRelativePosition;
	Quaternion									mRelativeQuaternion;
	XMFLOAT3									mScale;

	Position									mAccumulatedPosition;
	Quaternion									mAccumulatedQuaternion;
};