#include "RigidBody.h"
#include "Game.h"
#include <algorithm>


vector<RigidBody*> RigidBody::mRigidBodies;

RigidBody::RigidBody(Model* model):
	mModel(model)
{
	mVelocity = { 0.0f,-1.0f,0.0f };

	mRigidBodies.push_back(this);

	printf("Vertex Size : %d\n", mModel->mVertexBufferSize);

	CreateParticles();

	printf("Number of Partilces : %d\n", mParticles.size());
}

RigidBody::~RigidBody()
{
	int beforeCnt = mRigidBodies.size();
	mRigidBodies.erase(find(mRigidBodies.cbegin(), mRigidBodies.cend(), this));
	printf("erase rigid body [total rigid body :  %d->%d]\n",beforeCnt,mRigidBodies.size());
}

void RigidBody::SetVelocity(const XMFLOAT3& velocity)
{
	mVelocity = velocity;
}

void RigidBody::SetVelocity(const float& x, const float& y, const float& z)
{
	mVelocity = { x,y,z };
}

const XMFLOAT3& RigidBody::GetVelocity() const
{
	return mVelocity;
}

void RigidBody::CreateParticles()
{
	//model voxelization
	//depth peeling

	XMFLOAT3 center = mModel->mBound.Center;
	XMFLOAT3 extent = mModel->mBound.Extents;
	XMFLOAT4 orientation = mModel->mBound.Orientation;

	printf("center : %f %f %f\n", center.x, center.y, center.z);
	printf("extent : %f %f %f\n", extent.x, extent.y, extent.z);
	printf("orientation : %f %f %f %f\n", orientation.x, orientation.y, orientation.z, orientation.w);

	vector<Vertex>& vertices = mModel->mVertices;
	vector<uint16_t>& indices = mModel->mIndices;

	XMFLOAT3 velocity = GetVelocity();

	//삼각형 하나를 하나의 particle로 변환
	
	for (int i = 0; i < indices.size(); i += 3)
	{
		int a = indices[i];
		int b = indices[i + 1];
		int c = indices[i + 2];

		auto ap = XMLoadFloat3(&vertices[a].position);
		auto bp = XMLoadFloat3(&vertices[b].position);
		auto cp = XMLoadFloat3(&vertices[c].position);
		XMFLOAT3 average;

		XMStoreFloat3(&average, (ap + bp + cp) / 3);

		mParticles.push_back({
			{ average.x,average.y,average.z },
			velocity
		});
	}
	
}

void RigidBody::Draw()
{

}

void RigidBody::Collision()
{
	
}

void RigidBody::DrawParticles()
{

}