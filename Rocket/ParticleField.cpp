#include "ParticleField.h"
#include "Engine.h"

ParticleField::ParticleField()
{
	mParticles.push_back({ {50.0f,0.0f,50.0f} ,{15.0f,16.0f,17.0f} });
	mParticles.push_back({ {50.0f,0.0f,50.0f} ,{15.0f,9.0f,13.0f} });
	mParticles.push_back({ {50.0f,0.0f,50.0f} ,{-10.0f,15.0f,-13.0f} });
	mParticles.push_back({ {50.0f,0.0f,50.0f} ,{10.0f,5.0f,0.0f} });
	mParticles.push_back({ {50.0f,0.0f,50.0f} ,{-13.0f,5.0f,-12.0f} });
	mParticles.push_back({ {50.0f,0.0f,50.0f} ,{0.0f,15.0f,3.0f} });
	mParticles.push_back({ {50.0f,0.0f,50.0f} ,{-17.0f,15.0f,3.0f} });
	mParticles.push_back({ {50.0f,0.0f,50.0f} ,{10.0f,20.0f,-19.0f} });
	mParticles.push_back({ {50.0f,0.0f,50.0f} ,{-3.0f,10.0f,-10.0f} });
	mParticles.push_back({ {50.0f,0.0f,50.0f} ,{-7.0f,10.0f,18.0f} });
	mParticles.push_back({ {50.0f,0.0f,50.0f} ,{0.0f,10.0f,-10.0f} });
	mParticles.push_back({ {50.0f,0.0f,50.0f} ,{-6.0f,10.0f,18.0f} });
	mParticles.push_back({ {50.0f,0.0f,50.0f} ,{9.0f,23.0f,5.0f} });
	mParticles.push_back({ {50.0f,0.0f,50.0f} ,{16.0f,10.0f,-5.0f} });

	//constant buffer size�� ������ ������ copy�Ҷ� index 0���� ũ�� ������ ���� �� ����. �ϴ��� �ӽ÷� �̷��� �س���.
	mBuffer = make_unique<UploadBuffer>(Engine::mDevice.Get(), BufferInterface::ConstantBufferByteSize(sizeof(Particle)*mParticles.size()));
}

void ParticleField::Update(const Timer& timer)
{
	//particle �̵�
	for (auto p = mParticles.begin(); p != mParticles.end(); p++)
	{
		auto velocity = XMLoadFloat3(&p->mVelocity);
		auto position = XMLoadFloat3(&p->mPosition);
		position = XMVectorAdd(position, velocity * timer.GetDeltaTime());
		float x = XMVectorGetX(position);
		float y = XMVectorGetY(position);
		float z = XMVectorGetZ(position);
		if (x > 100.0f)
		{
			position = XMVectorSetX(position, 0.0f);
		}
		if (y > 100.0f)
		{
			position = XMVectorSetY(position, 0.0f);
		}
		if (z > 100.0f)
		{
			position = XMVectorSetZ(position, 0.0f);
		}

		XMStoreFloat3(&p->mPosition,position);
	}

	//buffer�� ī��
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