RWTexture2DArray<float4> particlePosMap : register(u1);

RWTexture3D<float4> rigidPosMap : register(u3);

RWTexture3D<float4> rigidQuatMap : register(u4);

RWTexture3D<float4> rigidLMMap : register(u5);

RWTexture3D<float4> rigidAMMap : register(u6);

RWTexture2D<int> mRigidInfos : register(u7);

RWTexture2D<float4> rigidInertia : register(u9);

RWTexture2D<float4> ForceMap : register(u10);

RWTexture3D<float4> rigidPosWriteMap : register(u11);

RWTexture3D<float4> rigidQuatWrtieMap : register(u12);

cbuffer constant : register(b0)
{
	int objNum;
	float deltaTime;
}

float4 QuatMul(float4 Q1, float4 Q2)
{
	float4 result;
	result.w = Q1.w * Q2.w - Q1.x * Q2.x - Q1.y * Q2.y - Q1.z * Q2.z;
	result.x = Q1.w * Q2.x + Q1.x * Q2.w + Q1.y * Q2.z - Q1.z * Q2.y;
	result.y = Q1.w * Q2.y - Q1.x * Q2.z + Q1.y * Q2.w + Q1.z * Q2.x;
	result.z = Q1.w * Q2.z + Q1.x * Q2.y - Q1.y * Q2.x + Q1.z * Q2.w;

	return result;
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
	int inertiaIdx = id * 6 + 3;
	/*
	* position을 계산하는 부분
	*/
	/*
	* particle당 질량이 1이므로 particle의 개수가 rigid body의 질량이다.
	*/
	int particleNum = mRigidInfos.Load(int2(rigidIdx % 128, rigidIdx / 128));

	float3 linearMomentum = rigidLMMap.Load(int3(id % 128, id / 128, 1)).xyz;

	float3 velocity = linearMomentum / (float)particleNum;

	float3 deltaPos = velocity* deltaTime;

	rigidPosWriteMap[int3(id % 128, id / 128, 0)] = rigidPosMap[int3(id % 128, id / 128, 0)] + float4(deltaPos,0.0f);

	/*
	* quaternion을 계산하는 부분
	*/
	float3 inertia0 = rigidInertia.Load(int3(inertiaIdx % 128, inertiaIdx / 128, 0)).xyz;
	inertiaIdx += 1;
	float3 inertia1 = rigidInertia.Load(int3(inertiaIdx % 128, inertiaIdx / 128, 0)).xyz;
	inertiaIdx += 1;
	float3 inertia2 = rigidInertia.Load(int3(inertiaIdx % 128, inertiaIdx / 128, 0)).xyz;

	float3x3 inverseInertiaMatrix = float3x3(
		inertia0.x, inertia0.y, inertia0.z,
		inertia1.x, inertia1.y, inertia1.z,
		inertia2.x, inertia2.y, inertia2.z
		);

	float3 angularMomentum = rigidAMMap.Load(int3(id % 128, id / 128, 1)).xyz;

	float3 angularVelocity = mul(inverseInertiaMatrix, angularMomentum);

	float angle = length(angularVelocity * deltaTime)/2.0f;

	float4 deltaQuat = float4(normalize(angularVelocity) * sin(angle), cos(angle));

	rigidQuatWrtieMap[int3(id % 128, id / 128, 0)] = QuatMul(deltaQuat, rigidQuatMap[int3(id % 128, id / 128, 0)]);
}