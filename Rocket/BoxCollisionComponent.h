#pragma once

#include "CollisionComponent.h"
#include "Constant.h"

class SceneNode;

class BoxCollisionComponent : public CollisionComponent
{
public:
												BoxCollisionComponent(shared_ptr<SceneNode> NodeAttachedTo,float width,float height,float depth);
	/*
	* other과 충돌이 있는 경우 true, 아니면 false를 반환한다.
	*/
	virtual bool								IsColliding(CollisionComponent* other, CollisionInfo& collisionInfo) override;
	/*
	* box collider를 그린다.
	*/
	virtual void								Draw() override;
	/*
	* box collider의 너비
	*/
	float										mWidth;
	/*
	* box collider의 높이
	*/
	float										mHeight;
	/*
	* box collider의 깊이
	*/
	float										mDepth;



protected:

	/*
	* GJK 알고리즘으로 충돌을 감지한다.
	*/
	bool										GJK(const vector<Vector3>& selfPoints, const vector<Vector3>& otherPoints,CollisionInfo& collisionInfo);
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
	void										EPA(Points& a, Points& b, Points& c, Points& d, const vector<Vector3>& selfPoints, const vector<Vector3>& otherPoints, CollisionInfo& collisionInfo);
	/*
	* serach Direction을 입력받아서 search point를 계산한다.
	*/
	void										CalculateSearchPoint(Points& point, Vector3& searchDir, const vector<Vector3>& selfPoints, const vector<Vector3>& otherPoints);
	/*
	* support function이다.
	*/
	Vector3										Support(const Vector3& dir, const vector<Vector3>& Points);
	/*
	* bary centric 좌표를 계산한다.
	*/
	void										Barycentric(const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& p, float& u, float& v, float& w);
	/*
	* vertex buffer view를 반환한다.
	*/
	D3D12_VERTEX_BUFFER_VIEW*					GetVertexBufferView();
	/*
	* index buffer view를 반환한다.
	*/
	D3D12_INDEX_BUFFER_VIEW*					GetIndexBufferView();

	/*
	* collider의 위치이다.
	*/
	Vector3										mSelfPosition;
	/*
	* 충돌을 계산하고자하는 상대 collider의 위치이다.
	*/
	Vector3										mOtherPosition;

	int											mIsColliding = 0;
	
	/*
	* vertex buffer의 idx이다.
	*/
	int											mVertexBufferIdx;
	/*
	* index buffer의 idx이다.
	*/
	int											mIndexBufferIdx;
	/*
	* vertex buffer에 업로드할 때 사용되는 upload buffer의 idx이다.
	*/
	int											mVertexUploadBufferIdx;
	/*
	* index buffer를 업로드할 때 사용되는 upload buffer의 idx이다.
	*/
	int											mIndexUploadBufferIdx;

	/*
	* box collider를 그릴때 필요한 vertex 정보이다.
	*/
	vector<Vertex>								mVertices;
	/*
	* box collider를 그릴때 필요한 index 정보이다.
	*/
	vector<uint16_t>							mIndices;
	/*
	* vertex buffer view의 정보이다.
	*/
	D3D12_VERTEX_BUFFER_VIEW					mVertexBufferView;
	/*
	* index buffer view의 정보이다.
	*/
	D3D12_INDEX_BUFFER_VIEW						mIndexBufferView;

};

