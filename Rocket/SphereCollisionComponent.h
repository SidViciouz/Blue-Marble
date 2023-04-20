#pragma once

#include "CollisionComponent.h"

class SphereCollisionComponent : public CollisionComponent
{
public:
												SphereCollisionComponent(shared_ptr<SceneNode> NodeAttachedTo, float radius);
	/*
	* other�� �浹�� �ִ� ��� true, �ƴϸ� false�� ��ȯ�Ѵ�.
	*/
	virtual bool								IsColliding(CollisionComponent* other, CollisionInfo& collisionInfo) override;
	/*
	* sphere collider�� �׸���.
	*/
	virtual void								Draw() override;
	/*
	* sphere collider�� ������
	*/
	float										mRadius;

protected:
	/*
	* GJK �˰������� �浹�� �����Ѵ�.
	*/
	bool										GJK(const vector<Vector3>& otherPoints, CollisionInfo& collisionInfo);
	/*
	* 3���� ���� ���� simplex�� ������Ʈ�Ѵ�.
	*/
	void										UpdateSimplex3(Points& a, Points& b, Points& c, Points& d, int& simpDim, Vector3& searchDir);
	/*
	* 4���� ���� ���� simplex�� ������Ʈ�Ѵ�. �浹�� �����ϴ� ��쿡�� true�� ��ȯ�Ѵ�.
	*/
	bool										UpdateSimplex4(Points& a, Points& b, Points& c, Points& d, int& simpDim, Vector3& searchDir);
	/*
	* EPA �˰������� �浹�� ���� ������ ��´�.
	*/
	void										EPA(Points& a, Points& b, Points& c, Points& d, const vector<Vector3>& otherPoints, CollisionInfo& collisionInfo);
	/*
	* serach Direction�� �Է¹޾Ƽ� search point�� ����Ѵ�.
	*/
	void										CalculateSearchPoint(Points& point, Vector3& searchDir, const vector<Vector3>& otherPoints);
	/*
	* support function�̴�.
	*/
	Vector3										Support(const Vector3& dir, const vector<Vector3>& Points);
	/*
	* bary centric ��ǥ�� ����Ѵ�.
	*/
	void										Barycentric(const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& p, float& u, float& v, float& w);
	/*
	* collider�� ��ġ�̴�.
	*/
	Vector3										mSelfPosition;
	/*
	* �浹�� ����ϰ����ϴ� ��� collider�� ��ġ�̴�.
	*/
	Vector3										mOtherPosition;
};
