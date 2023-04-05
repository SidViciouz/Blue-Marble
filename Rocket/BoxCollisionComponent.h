#pragma once

#include "CollisionComponent.h"
#include "Constant.h"

class SceneNode;

class BoxCollisionComponent : public CollisionComponent
{
public:
												BoxCollisionComponent(shared_ptr<SceneNode> NodeAttachedTo,float width,float height,float depth);
	virtual bool								IsColliding(CollisionComponent* other, CollisionInfo& collisionInfo) override;
	virtual void								Draw() override;

	float										mWidth;
	float										mHeight;
	float										mDepth;



protected:

	bool										GJK(const vector<Vector3>& selfPoints, const vector<Vector3>& otherPoints,CollisionInfo& collisionInfo);
	void										UpdateSimplex3(Points& a, Points& b, Points& c, Points& d, int& simpDim, Vector3& searchDir);
	bool										UpdateSimplex4(Points& a, Points& b, Points& c, Points& d, int& simpDim, Vector3& searchDir);
	void										EPA(Points& a, Points& b, Points& c, Points& d, const vector<Vector3>& selfPoints, const vector<Vector3>& otherPoints, CollisionInfo& collisionInfo);
	void										CalculateSearchPoint(Points& point, Vector3& searchDir, const vector<Vector3>& selfPoints, const vector<Vector3>& otherPoints);
	Vector3										Support(const Vector3& dir, const vector<Vector3>& Points);
	void										Barycentric(const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& p, float& u, float& v, float& w);

	D3D12_VERTEX_BUFFER_VIEW*					GetVertexBufferView();
	D3D12_INDEX_BUFFER_VIEW*					GetIndexBufferView();

	Vector3										mSelfPosition;
	Vector3										mOtherPosition;
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

