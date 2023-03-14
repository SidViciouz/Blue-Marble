RWTexture2DArray<float4> particleCOMMap : register(u0);

RWTexture2DArray<float4> particleVelMap : register(u2);

RWTexture2DArray<float4> rigidLMMap : register(u5);

RWTexture2DArray<float4> rigidAMMap : register(u6);

RWTexture2D<float4> rigidInertia : register(u9);
/*
* particle °³¼ö, rigidbodyÀÇ offset
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
		int particleNumIdx = 2 * i;
		int offsetIdx = 2 * i + 1;

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

	int inertiaIdx = rigidBodyIndex * 6 + 3;
	float3 inertia0 = rigidInertia.Load(int3(inertiaIdx % 128,inertiaIdx / 128,0)).xyz;
	inertiaIdx += 1;
	float3 inertia1 = rigidInertia.Load(int3(inertiaIdx % 128, inertiaIdx / 128, 0)).xyz;
	inertiaIdx += 1;
	float3 inertia2 = rigidInertia.Load(int3(inertiaIdx % 128, inertiaIdx / 128, 0)).xyz;
	float3x3 inverseInnertiaM = float3x3(inertia0,inertia1,inertia2);

	float4 velocity = rigidLM / rigidParticleNum;
	float4 velocityFromAngular = float4(mul(inverseInnertiaM,rigidAM.xyz)* particleCOMPos.xyz, 0.0f);

	particleVelMap[int3(particleIdxX, particleIdxY, 0)] = velocity + velocityFromAngular;
}