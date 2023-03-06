#include "ParticleField.h"
#include "Pipeline.h"

ParticleField::ParticleField()
{
	mParticles.push_back({ {0.0f,0.0f,0.0f} ,{0.5f,0.0f,0.0f} });
	mParticles.push_back({ {0.0f,1.0f,0.0f} ,{0.5f,0.0f,0.0f} });
	mParticles.push_back({ {0.0f,2.0f,0.0f} ,{0.1f,0.5f,0.3f} });
	mParticles.push_back({ {0.0f,2.0f,3.0f} ,{0.0f,0.5f,0.0f} });
	mParticles.push_back({ {0.0f,2.0f,5.0f} ,{0.0f,0.5f,0.2f} });
	mParticles.push_back({ {0.0f,2.0f,8.0f} ,{0.0f,0.5f,0.3f} });
	mParticles.push_back({ {0.0f,0.0f,0.0f} ,{0.7f,0.5f,0.3f} });

	//constant buffer size로 만들지 않으면 copy할때 index 0보다 크면 문제가 생길 것 같다. 일단은 임시로 이렇게 해놓음.
	mBuffer = make_unique<UploadBuffer>(Pipeline::mDevice.Get(), BufferInterface::ConstantBufferByteSize(sizeof(Particle)*mParticles.size()));
}

void ParticleField::Update(const Timer& timer)
{
	//particle 이동
	for (auto p = mParticles.begin(); p != mParticles.end(); p++)
	{
		auto velocity = XMLoadFloat3(&p->mVelocity);
		auto position = XMLoadFloat3(&p->mPosition);
		position = XMVectorAdd(position, velocity * timer.GetDeltaTime());
		float x = XMVectorGetX(position);
		float y = XMVectorGetY(position);
		float z = XMVectorGetZ(position);
		if (x > 10.0f)
		{
			position = XMVectorSetX(position, 0.0f);
		}
		if (y > 10.0f)
		{
			position = XMVectorSetY(position, 0.0f);
		}
		if (z > 10.0f)
		{
			position = XMVectorSetZ(position, 0.0f);
		}

		XMStoreFloat3(&p->mPosition,position);
	}

	//buffer에 카피
	mBuffer->Copy(0, mParticles.data(), sizeof(Particle) * mParticles.size());
}

D3D12_VERTEX_BUFFER_VIEW* ParticleField::GetVertexBufferView()
{
	mVertexBufferView.BufferLocation = mBuffer->GetGpuAddress();
	mVertexBufferView.SizeInBytes = sizeof(Particle) * mParticles.size();
	mVertexBufferView.StrideInBytes = sizeof(Particle);

	return &mVertexBufferView;
}

int ParticleField:: NumParticle()
{
	return mParticles.size();
}