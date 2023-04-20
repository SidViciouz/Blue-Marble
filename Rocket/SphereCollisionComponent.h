#pragma once

#include "CollisionComponent.h"

class SphereCollisionComponent : public CollisionComponent
{
public:
												SphereCollisionComponent(shared_ptr<SceneNode> NodeAttachedTo, float radius);
	/*
	* other과 충돌이 있는 경우 true, 아니면 false를 반환한다.
	*/
	virtual bool								IsColliding(CollisionComponent* other, CollisionInfo& collisionInfo) override;
	/*
	* sphere collider를 그린다.
	*/
	virtual void								Draw() override;
	/*
	* sphere collider의 반지름
	*/
	float										mRadius;

protected:
	/*
	* GJK 알고리즘으로 충돌을 감지한다.
	*/
	bool										GJK(const vector<Vector3>& otherPoints, CollisionInfo& collisionInfo);
	/*
	* 3개의 점을 가진 simplex를 업데이트한다.
	*/
	void										UpdateSimplex3(Points& a, Points& b, Points& c, Points& d, int& simpDim, Vector3& searchDir);
	/*
	* 4개의 점을 가진 simplex를 업데이트한다. 충돌을 감지하는 경우에는 true를 반환한다.
	*/
	bool										UpdateSimplex4(Points& a, Points& b, Points& c, Points& d, int& simpDim, Vector3& searchDir);
	/*
	* EPA 알고리즘으로 충돌에 대한 정보를 얻는다.
	*/
	void										EPA(Points& a, Points& b, Points& c, Points& d, const vector<Vector3>& otherPoints, CollisionInfo& collisionInfo);
	/*
	* serach Direction을 입력받아서 search point를 계산한다.
	*/
	void										CalculateSearchPoint(Points& point, Vector3& searchDir, const vector<Vector3>& otherPoints);
	/*
	* support function이다.
	*/
	Vector3										Support(const Vector3& dir, const vector<Vector3>& Points);
	/*
	* bary centric 좌표를 계산한다.
	*/
	void										Barycentric(const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& p, float& u, float& v, float& w);
	/*
	* collider의 위치이다.
	*/
	Vector3										mSelfPosition;
	/*
	* 충돌을 계산하고자하는 상대 collider의 위치이다.
	*/
	Vector3										mOtherPosition;
};
