#pragma once

#include "Constant.h"
#include "InputComponentBase.h"
#include "../Maths/Quaternion.h"

using namespace Maths;

class Scene;

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
	virtual void								SetRelativePosition(const XMFLOAT3& position);
	virtual void								SetRelativePosition(const float& x,const float& y, const float& z);
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
	* 이 노드의 global quaternion을 설정한다.
	*/
	void										SetAccumulatedQuaternion(const XMFLOAT4& quaternion);
	void										SetAccumulatedQuaternion(const float& x, const float& y, const float& z, const float& w);
	/*
	* 상대적인 위치에 입력으로 들어온 크기만큼 더한다.
	*/
	void										AddRelativePosition(const XMFLOAT3& position);
	void										AddRelativePosition(const float& x, const float& y, const float& z);

	/*
	* 상대적인 위치에 d*position을 더한다.
	*/
	void										MulAddRelativePosition(const float& d,const XMFLOAT3& position);
	/*
	* 상대적인 위치를 반환한다.
	*/
	Vector3										GetRelativePosition();
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
	Vector3										GetAccumulatedPosition();
	/*
	* global quaternion을 반환한다.
	*/
	Quaternion									GetAccumulatedQuaternion();
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
	/*
	* 이 노드가 속한 Scene을 설정한다.
	*/
	void										SetScene(Scene* scene);
	/*
	* 이 노드가 속한 Scene을 반환한다.
	*/
	Scene*										GetScene() const;
	/*
	* diffuse albedo를 설정한다.
	*/
	void										SetDiffuseAlbedo(const float& x, const float& y, const float& z);
	/*
	* roughness를 설정한다.
	*/
	void										SetRoughness(const float& roughness);
	/*
	* Fresnel값을 설정한다.
	*/
	void										SetFresnel(const float& x, const float& y, const float& z);

protected:
	/*
	* 부모 노드와의 상대적인 position이다.
	*/
	Vector3										mRelativePosition;
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
	Vector3										mAccumulatedPosition;
	/*
	* global quaternion을 나타낸다.
	*/
	Quaternion									mAccumulatedQuaternion;
	/*
	* 이 노드가 속한 Scene을 가리킨다. 
	*/
	Scene*										mScene;
};