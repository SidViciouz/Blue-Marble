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
	* S�� simplex�� point���� �����̰�,
	* F�� �� point��κ��� face�� �����ϴ� �����̴�. ���ӵ� 3���� �ε����� �ϳ��� face�� �����Ѵ�.
	* face�� counter clockwise�� ���ǵǾ����.
	*/
	Vector3										GetClosestPoints(const vector<Vector3>& S, const vector<int>& F) const;
	/*
	* ���ο� point N�� S�� �߰��ϰ�, �̸� �̿��ؼ� F�� ���Ӱ� �����Ѵ�.
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

