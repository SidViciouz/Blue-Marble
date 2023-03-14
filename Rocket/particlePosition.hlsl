RWTexture2DArray<float4> particleCOMMap : register(u0);

RWTexture2DArray<float4> particlePosMap : register(u1);

RWTexture2DArray<float4> rigidPosMap : register(u3);

RWTexture2DArray<float4> rigidQuatMap : register(u4);

/*
* particle °³¼ö, rigidbodyÀÇ offset
*/
RWTexture2D<int> mRigidInfos : register(u7);

cbuffer constant : register(b0)
{
	int objNum;
}

float4 RotationWithQuaternion(float4 Q, float4 P)
{
	float3x3 R = {
		1.0f - 2.0f * Q.y * Q.y - 2.0f * Q.z * Q.z, 2.0f * Q.x * Q.y + 2.0f * Q.z * Q.w, 2.0f * Q.x * Q.z - 2.0f * Q.y * Q.w,
		2.0f * Q.x * Q.y - 2.0f * Q.z * Q.w, 1.0f - 2.0f * Q.x * Q.x - 2.0f * Q.z * Q.z, 2.0f * Q.y * Q.z + 2.0f * Q.x * Q.w,
		2.0f * Q.x * Q.z + 2.0f * Q.y * Q.w, 2.0f * Q.y * Q.x - 2.0f * Q.x * Q.w, 1.0f - 2.0f * Q.x * Q.x - 2.0f * Q.y * Q.y
	};

	return float4(mul(P.xyz, R),0.0f);
}

[numthreads(1024, 1, 1)]
void CS( int id : SV_GroupIndex)
{
	int rigidBodyIndex = -1;

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
			break;
		}
	}

	if (i >= objNum)
		return;

	float4 rigidPos = rigidPosMap.Load(int4(rigidBodyIndex%128,rigidBodyIndex/128,0,0));
	float4 rigidQuat = rigidQuatMap.Load(int4(rigidBodyIndex % 128, rigidBodyIndex / 128, 0, 0));
	float4 particleCOMPos = particleCOMMap.Load(int4(particleIdxX, particleIdxY, 0,0));

	particlePosMap[int3(particleIdxX, particleIdxY, 0)] = rigidPos + RotationWithQuaternion(rigidQuat, particleCOMPos);
}
