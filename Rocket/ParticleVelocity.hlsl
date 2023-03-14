RWTexture2DArray<float4> particleCOMMap : register(u0);

RWTexture2DArray<float4> particleVelMap : register(u2);

RWTexture2DArray<float4> rigidLMMap : register(u5);

RWTexture2DArray<float4> rigidAMMap : register(u6);

/*
* particle 개수, rigidbody의 offset
*/
RWTexture2D<int> mRigidInfos : register(u7);

cbuffer constant : register(b0)
{
	int objNum;
}

[numthreads(1024, 1, 1)]
void CS(int id : SV_GroupIndex)
{
	int rigidBodyIndex = -1;
	int rigidParticleNum = -1;

	int particleIdxX = id % 128;
	int particleIdxY = id / 128;

	int i = 0;

	for (i = 0; i < objNum; ++i)
	{
		int particleNumIdx = 3 * i;
		int offsetIdx = 3 * i + 1;

		int offset = mRigidInfos[int2(offsetIdx % 128, offsetIdx / 128)];
		int particleNum = mRigidInfos[int2(particleNumIdx % 128, particleNumIdx / 128)];

		if (offset <= id && id < offset + particleNum)
		{
			rigidBodyIndex = i;
			rigidParticleNum = particleNum;
			break;
		}
	}

	if (i >= objNum)
		return;

	float4 rigidLM = rigidLMMap.Load(int4(rigidBodyIndex % 128, rigidBodyIndex / 128, 0, 0));
	float4 rigidAM = rigidAMMap.Load(int4(rigidBodyIndex % 128, rigidBodyIndex / 128, 0, 0));
	float4 particleCOMPos = particleCOMMap.Load(int4(particleIdxX, particleIdxY, 0, 0));

	particleVelMap[int3(particleIdxX, particleIdxY, 0)] = rigidLM / rigidParticleNum; // angular momemtum을 이용해서 얻은 velocity 항을 추가해야한다.
}