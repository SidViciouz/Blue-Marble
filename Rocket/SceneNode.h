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
	virtual void								SetRelativePosition(const XMFLOAT3& position);
	virtual void								SetRelativePosition(const float& x,const float& y, const float& z);
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
	* �� ����� global quaternion�� �����Ѵ�.
	*/
	void										SetAccumulatedQuaternion(const XMFLOAT4& quaternion);
	void										SetAccumulatedQuaternion(const float& x, const float& y, const float& z, const float& w);
	/*
	* ������� ��ġ�� �Է����� ���� ũ�⸸ŭ ���Ѵ�.
	*/
	void										AddRelativePosition(const XMFLOAT3& position);
	void										AddRelativePosition(const float& x, const float& y, const float& z);

	/*
	* ������� ��ġ�� d*position�� ���Ѵ�.
	*/
	void										MulAddRelativePosition(const float& d,const XMFLOAT3& position);
	/*
	* ������� ��ġ�� ��ȯ�Ѵ�.
	*/
	Vector3										GetRelativePosition();
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
	Vector3										GetAccumulatedPosition();
	/*
	* global quaternion�� ��ȯ�Ѵ�.
	*/
	Quaternion									GetAccumulatedQuaternion();
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
	/*
	* �� ��尡 ���� Scene�� �����Ѵ�.
	*/
	void										SetScene(Scene* scene);
	/*
	* �� ��尡 ���� Scene�� ��ȯ�Ѵ�.
	*/
	Scene*										GetScene() const;
	/*
	* diffuse albedo�� �����Ѵ�.
	*/
	void										SetDiffuseAlbedo(const float& x, const float& y, const float& z);
	/*
	* roughness�� �����Ѵ�.
	*/
	void										SetRoughness(const float& roughness);
	/*
	* Fresnel���� �����Ѵ�.
	*/
	void										SetFresnel(const float& x, const float& y, const float& z);

protected:
	/*
	* �θ� ������ ������� position�̴�.
	*/
	Vector3										mRelativePosition;
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
	Vector3										mAccumulatedPosition;
	/*
	* global quaternion�� ��Ÿ����.
	*/
	Quaternion									mAccumulatedQuaternion;
	/*
	* �� ��尡 ���� Scene�� ����Ų��. 
	*/
	Scene*										mScene;
};