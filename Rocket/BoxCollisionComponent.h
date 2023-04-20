#pragma once

#include "CollisionComponent.h"
#include "Constant.h"

class SceneNode;

class BoxCollisionComponent : public CollisionComponent
{
public:
												BoxCollisionComponent(shared_ptr<SceneNode> NodeAttachedTo,float width,float height,float depth);
	/*
	* other�� �浹�� �ִ� ��� true, �ƴϸ� false�� ��ȯ�Ѵ�.
	*/
	virtual bool								IsColliding(CollisionComponent* other, CollisionInfo& collisionInfo) override;
	/*
	* box collider�� �׸���.
	*/
	virtual void								Draw() override;
	/*
	* box collider�� �ʺ�
	*/
	float										mWidth;
	/*
	* box collider�� ����
	*/
	float										mHeight;
	/*
	* box collider�� ����
	*/
	float										mDepth;



protected:

	/*
	* GJK �˰������� �浹�� �����Ѵ�.
	*/
	bool										GJK(const vector<Vector3>& selfPoints, const vector<Vector3>& otherPoints,CollisionInfo& collisionInfo);
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
	void										EPA(Points& a, Points& b, Points& c, Points& d, const vector<Vector3>& selfPoints, const vector<Vector3>& otherPoints, CollisionInfo& collisionInfo);
	/*
	* serach Direction�� �Է¹޾Ƽ� search point�� ����Ѵ�.
	*/
	void										CalculateSearchPoint(Points& point, Vector3& searchDir, const vector<Vector3>& selfPoints, const vector<Vector3>& otherPoints);
	/*
	* support function�̴�.
	*/
	Vector3										Support(const Vector3& dir, const vector<Vector3>& Points);
	/*
	* bary centric ��ǥ�� ����Ѵ�.
	*/
	void										Barycentric(const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& p, float& u, float& v, float& w);
	/*
	* vertex buffer view�� ��ȯ�Ѵ�.
	*/
	D3D12_VERTEX_BUFFER_VIEW*					GetVertexBufferView();
	/*
	* index buffer view�� ��ȯ�Ѵ�.
	*/
	D3D12_INDEX_BUFFER_VIEW*					GetIndexBufferView();

	/*
	* collider�� ��ġ�̴�.
	*/
	Vector3										mSelfPosition;
	/*
	* �浹�� ����ϰ����ϴ� ��� collider�� ��ġ�̴�.
	*/
	Vector3										mOtherPosition;

	int											mIsColliding = 0;
	
	/*
	* vertex buffer�� idx�̴�.
	*/
	int											mVertexBufferIdx;
	/*
	* index buffer�� idx�̴�.
	*/
	int											mIndexBufferIdx;
	/*
	* vertex buffer�� ���ε��� �� ���Ǵ� upload buffer�� idx�̴�.
	*/
	int											mVertexUploadBufferIdx;
	/*
	* index buffer�� ���ε��� �� ���Ǵ� upload buffer�� idx�̴�.
	*/
	int											mIndexUploadBufferIdx;

	/*
	* box collider�� �׸��� �ʿ��� vertex �����̴�.
	*/
	vector<Vertex>								mVertices;
	/*
	* box collider�� �׸��� �ʿ��� index �����̴�.
	*/
	vector<uint16_t>							mIndices;
	/*
	* vertex buffer view�� �����̴�.
	*/
	D3D12_VERTEX_BUFFER_VIEW					mVertexBufferView;
	/*
	* index buffer view�� �����̴�.
	*/
	D3D12_INDEX_BUFFER_VIEW						mIndexBufferView;

};

