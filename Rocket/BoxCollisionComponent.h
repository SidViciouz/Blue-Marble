#pragma once

#include "CollisionComponent.h"
#include "Constant.h"

class SceneNode;

class BoxCollisionComponent : public CollisionComponent
{
public:
												BoxCollisionComponent(shared_ptr<SceneNode> NodeAttachedTo,int width,int height,int depth);
	virtual bool								IsColliding(CollisionComponent* other) override;
	virtual void								Draw() override;

	int											mWidth;
	int											mHeight;
	int											mDepth;

	struct Closest
	{
		Vector3 closestPoint;
		int		closestPlaneIdx;					
	};

protected:

	Vector3										Support(const Vector3& D, const vector<Vector3>& selfPoints, const vector<Vector3>& otherPoints) const;
	bool										DoSimplex(vector<Vector3>& S, Vector3& D) const;
	CollisionInfo								EPA(vector<Vector3>& S,const vector<Vector3>& selfPoints,const vector<Vector3>& otherPoints);
	/*
	* S는 simplex의 point들의 집합이고,
	* F는 그 point들로부터 face를 정의하는 집합이다. 연속된 3개의 인덱스가 하나의 face를 정의한다.
	* face는 counter clockwise로 정의되어었다.
	*/
	Vector3										GetClosestPoints(const vector<Vector3>& S, const vector<int>& F) const;
	/*
	* 새로운 point N을 S에 추가하고, 이를 이용해서 F를 새롭게 정의한다.
	*/
	bool										Expand(vector<Vector3>& S, vector<int>& F, const Vector3& N) const;

	Vector3										BaryCentric(const Vector3& p, const Vector3& a, const Vector3& b, const Vector3& c) const;

	D3D12_VERTEX_BUFFER_VIEW*					GetVertexBufferView();
	D3D12_INDEX_BUFFER_VIEW*					GetIndexBufferView();

	int											mIsColliding = 0;
	int											mVertexBufferIdx;
	int											mIndexBufferIdx;
	int											mVertexUploadBufferIdx;
	int											mIndexUploadBufferIdx;
	vector<Vertex>								mVertices;
	vector<uint16_t>							mIndices;
	D3D12_VERTEX_BUFFER_VIEW					mVertexBufferView;
	D3D12_INDEX_BUFFER_VIEW						mIndexBufferView;

};

