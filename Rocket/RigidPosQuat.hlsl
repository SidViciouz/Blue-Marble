RWTexture2DArray<float4> particlePosMap : register(u1);

RWTexture3D<float4> rigidPosMap : register(u3);

RWTexture3D<float4> rigidLMMap : register(u5);

RWTexture3D<float4> rigidAMMap : register(u6);

RWTexture2D<int> mRigidInfos : register(u7);

RWTexture2D<float4> ForceMap : register(u10);

cbuffer constant : register(b0)
{
	int objNum;
	float deltaTime;
}

/*
* id는 하나의 rigidbody를 나타낸다.
*/
[numthreads(128, 1, 1)]
void CS(uint id : SV_GroupIndex)
{
	if (id >= objNum)
		return;

	int rigidIdx = id * 2;

	/*
	* particle당 질량이 1이므로 particle의 개수가 rigid body의 질량이다.
	*/
	int particleNum = mRigidInfos.Load(int2(rigidIdx % 128, rigidIdx / 128));

	float3 linearMomentum = rigidLMMap.Load(int3(id % 128, id / 128, 1)).xyz;

	float3 velocity = linearMomentum / (float)particleNum;

	float3 deltaPos = velocity* deltaTime;

	rigidPosMap[int3(id % 128, id / 128, 1)] = rigidPosMap[int3(id % 128, id / 128, 0)] + float4(deltaPos,0.0f);
}