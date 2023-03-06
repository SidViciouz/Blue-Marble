#pragma once
#include "Util.h"
#include "Buffer.h"
#include <vector>

struct Particle
{
	XMFLOAT3 mPosition;
	XMFLOAT3 mVelocity;
};

class ParticleField
{
public:
	ParticleField();
	void Update();
	D3D12_VERTEX_BUFFER_VIEW GetView();

private:
	vector<Particle> mParticles;
	unique_ptr<UploadBuffer> mBuffer;
};
