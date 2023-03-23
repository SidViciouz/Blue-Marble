#include "RigidBody.h"
#include "RigidBodySystem.h"
#include "Engine.h"
#include <algorithm>

RigidBody::RigidBody(Model* model):
	mModel(model)
{
	SetLinearMomentum(1.0f, 1.0f, 1.0f);
	SetAngularMomentum(0.0f, 0.0f, 5.0f);

	RigidBodySystem::mRigidBodies.push_back(this);

	printf("Vertex Size : %d\n", mModel->mVertexBufferSize);

	CreateParticles();

	printf("Number of Partilces : %d\n", mParticles.size());
}

RigidBody::~RigidBody()
{
	int beforeCnt = RigidBodySystem::mRigidBodies.size();
	RigidBodySystem::mRigidBodies.erase(find(RigidBodySystem::mRigidBodies.cbegin(), RigidBodySystem::mRigidBodies.cend(), this));
	printf("erase rigid body [total rigid body :  %d->%d]\n", beforeCnt, RigidBodySystem::mRigidBodies.size());
}

void RigidBody::SetLinearMomentum(const XMFLOAT3& linearMomentum)
{
	mLinearMomentum = linearMomentum;
}

void RigidBody::SetLinearMomentum(const float& x, const float& y, const float& z)
{
	mLinearMomentum = { x,y,z };
}

const XMFLOAT3& RigidBody::GetLinearMomentum() const
{
	return mLinearMomentum;
}

void RigidBody::SetAngularMomentum(const XMFLOAT3& AngularMomentum)
{
	mAngularMomentum = AngularMomentum;
}
void RigidBody::SetAngularMomentum(const float& x, const float& y, const float& z)
{
	mAngularMomentum = { x,y,z };
}

const XMFLOAT3& RigidBody::GetAngularMomentum() const
{
	return mAngularMomentum;
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

	XMFLOAT3 velocity = GetLinearMomentum();

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
	//volume인 경우, vertex buffer가 없다. volume을 model을 상속하지 않도록 변경해야한다.
	if (mModel->mVertexBufferSize == 0)
		return;
	Engine::mCommandList->SetPipelineState(Engine::mPSOs["RigidParticle"].Get());
	Engine::mCommandList->SetGraphicsRootSignature(Engine::mRootSignatures["RigidParticle"].Get());
	Engine::mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	Engine::mCommandList->IASetVertexBuffers(0, 1, mModel->GetVertexBufferView());
	Engine::mCommandList->IASetIndexBuffer(mModel->GetIndexBufferView());
	Engine::mCommandList->DrawIndexedInstanced(mModel->mIndexBufferSize, 1, 0, 0, 0);
}