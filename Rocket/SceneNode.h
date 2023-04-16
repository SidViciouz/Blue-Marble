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
	* 이 노드를 먼저 그리고 자식 노드들을 그린다.
	*/
	virtual void								Draw();
	/*
	* pipeline setting을 하지 않고 이 노드와 자식 노드들을 그린다.
	*/
	virtual void								DrawWithoutSetting();
	/*
	* 프레임마다 데이터를 업데이트한다.
	*/
	virtual void								Update();
	/*
	* 이 노드의 collisionComponent가 상대 노드의 collisionComponent와 충돌하는지의 여부와 충돌정보를 반환한다.
	*/
	virtual bool								IsColliding(SceneNode* counterPart, CollisionInfo& collisionInfo);
	/*
	* 이 노드에 자식 노드를 추가한다.
	*/
	void										AddChild(shared_ptr<SceneNode> child);
	/*
	* 자식 노드를 삭제한다.
	*/
	void										RemoveChild(shared_ptr<SceneNode> child);
	/*
	* 부모 노드와의 상대적인 위치를 설정한다.
	*/
	void										SetRelativePosition(const XMFLOAT3& position);
	void										SetRelativePosition(const float& x,const float& y, const float& z);
	/*
	* 부모 노드와의 상대적인 quaternion을 설정한다.
	*/
	void										SetRelativeQuaternion(const XMFLOAT4& quaternion);
	void										SetRelativeQuaternion(const float& x,const float& y,const float& z,const float& w);
	/*
	* scale을 설정한다.
	*/
	void										SetScale(const XMFLOAT3& scale);
	void										SetScale(const float& x,const float& y, const float& z);
	/*
	* 이 노드의 global position을 설정한다.
	*/
	void										SetAccumulatedPosition(const XMFLOAT3& position);
	void										SetAccumulatedPosition(const float& x, const float& y, const float& z);
	/*
	* 상대적인 위치에 입력으로 들어온 크기만큼 더한다.
	*/
	void										AddRelativePosition(const XMFLOAT3& position);
	void										AddRelativePosition(const float& x, const float& y, const float& z);
	/*
	* 상대적인 quaternion에 입력으로 들어온 quaternion을 곱한다.
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
	* 상대적인 위치에 d*position을 더한다.
	*/
	void										MulAddRelativePosition(const float& d,const XMFLOAT3& position);
	/*
	* 상대적인 위치를 반환한다.
	*/
	Position									GetRelativePosition();
	/*
	* 상대적인 quaternion을 반환한다.
	*/
	Quaternion									GetRelativeQuaternion();
	/*
	* scale을 반환한다.
	*/
	XMFLOAT3									GetScale();
	/*
	* global position을 반환한다.
	*/
	Position									GetAccumulatedPosition();
	/*
	* global quaternion을 반환한다.
	*/
	Quaternion									GetAccumulatedQuaternion();
	/*
	* 이 노드에 연결한 collision component를 가리킨다.
	*/
	shared_ptr<CollisionComponent>				mCollisionComponent;
	/*
	* 이 노드에 연결한 rigid body component를 가리킨다.
	*/
	shared_ptr<RigidBodyComponent>				mRigidBodyComponent;
	/*
	* 이 노드에 연결한 input component를 가리킨다.
	*/
	shared_ptr<InputComponentBase>				mInputComponent;
	/*
	* 이 노드가 프레임마다 업로드할 데이터를 나타낸다.
	*/
	obj											mObjFeature;
	/*
	* 자식 노드들을 가리킨다.
	*/
	vector<shared_ptr<SceneNode>>				mChildNodes;
	/*
	* 부모 노드를 가리킨다.
	*/
	SceneNode*									mParentNode = nullptr;
	/*
	* node table내에서의 index를 가리킨다.
	*/
	int											mSceneNodeIndex;
	/*
	* 모든 node들을 담고있는 table이다.
	*/
	static bool									mSceneNodeNumTable[MAX_SCENE_NODE_NUM];

protected:
	/*
	* 부모 노드와의 상대적인 position이다.
	*/
	Position									mRelativePosition;
	/*
	* 부모 노드와의 상대적인 quaternion이다.
	*/
	Quaternion									mRelativeQuaternion;
	/*
	* scale을 나타낸다.
	*/
	XMFLOAT3									mScale;
	/*
	* global position을 나타낸다.
	*/
	Position									mAccumulatedPosition;
	/*
	* global quaternion을 나타낸다.
	*/
	Quaternion									mAccumulatedQuaternion;
};