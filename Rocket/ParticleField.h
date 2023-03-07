#pragma once
#include "Util.h"
#include "Buffer.h"
#include "Timer.h"
#include <vector>

struct Particle
{
	XMFLOAT3									mPosition;
	XMFLOAT3									mVelocity;
};

class ParticleField
{
public:
												ParticleField();
	void										Update(const Timer& timer);
	D3D12_VERTEX_BUFFER_VIEW*					GetVertexBufferView();
	int											NumParticle();

private:
	D3D12_VERTEX_BUFFER_VIEW					mVertexBufferView;
	vector<Particle>							mParticles;
	unique_ptr<UploadBuffer>					mBuffer;
};
