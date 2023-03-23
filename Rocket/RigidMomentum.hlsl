RWTexture2DArray<float4> particlePosMap : register(u1);

RWTexture3D<float4> rigidPosMap : register(u3);

RWTexture3D<float4> rigidLMMap : register(u5);

RWTexture3D<float4> rigidAMMap : register(u6);

RWTexture2D<int> mRigidInfos : register(u7);

RWTexture2D<float4> ForceMap : register(u10);

RWTexture3D<float4> rigidLMWriteMap : register(u13);

RWTexture3D<float4> rigidAMWriteMap : register(u14);

cbuffer constant : register(b0)
{
	float deltaTime;
}

groupshared float3 linearForce = 0.0f;
groupshared float3 angularForce = 0.0f;
groupshared int mutex = 0;
groupshared int mutexI = 0;
/*
* 125�� �ϳ��� ��ü�� ���� �� �ִ� �ִ� particle�� �����̴�.
* id�� �� particle�� index�̴�. groupId�� �� rigidBody�� index�̴�.
*/
[numthreads(125, 1, 1)]
void CS( uint id : SV_GroupIndex, uint3 groupId : SV_GroupID)
{
	int ori = 1;
	InterlockedCompareExchange(mutexI, 0, 1, ori);
	if (ori == 0)
	{
		rigidLMWriteMap[int3(groupId.x % 128, groupId.x / 128, 0)] = rigidLMMap[int3(groupId.x % 128, groupId.x / 128, 0)];
		rigidAMWriteMap[int3(groupId.x % 128, groupId.x / 128, 0)] = rigidAMMap[int3(groupId.x % 128, groupId.x / 128, 0)];
	}
	GroupMemoryBarrierWithGroupSync();

	int rigidIdx = groupId.x * 2;
	int particleNumber = mRigidInfos.Load(int2(rigidIdx % 128, rigidIdx / 128));
	rigidIdx += 1;
	int rigidOffset = mRigidInfos.Load(int2(rigidIdx % 128, rigidIdx / 128));

	//particle index�� 1���� �����ϵ��� �ٲٸ� id > particleNumber���� �����ؾ���.
	if (id >= particleNumber)
		return;

	int particleIdx = rigidOffset + id;

	float3 particleRelativePos = particlePosMap.Load(int4(particleIdx % 128, particleIdx / 128, 0, 0)).xyz
		- rigidPosMap.Load(int4(groupId.x % 128, groupId.x / 128, 0, 0)).xyz;

	float3 force = ForceMap.Load(int2(particleIdx % 128, particleIdx / 128)).xyz;
	float3 angularForce = cross(particleRelativePos,force);

	
	float3 particlePos = particlePosMap.Load(int4(particleIdx % 128, particleIdx / 128, 0, 0)).xyz;
	if (particlePos.x < 0)
	{
		force.x = 10.0f;
	}
	else if (particlePos.x > 31)
	{
		force.x = -10.0f;
	}
	if (particlePos.y < 0)
	{
		force.y = 13.0f;
	}
	else if (particlePos.y > 31)
	{
		force.y = -10.0f;
	}
	if (particlePos.z < 0)
	{
		force.z = 10.0f;
	}
	else if (particlePos.z > 31)
	{
		force.z = -10.0f;
	}

	bool locked = true;

	while (locked)
	{
		int original = 1;
		InterlockedCompareExchange(mutex, 0, 1, original);
		if (original == 0)
		{
			/*
			* force�� deltaTime�� ���ؼ� ����� ��´�.
			*/
			//linearForce += force;
			rigidLMWriteMap[int3(groupId.x % 128, groupId.x / 128, 0)] += float4(force, 0.0f) * deltaTime + float4(0.0f,-9.8f,0.0f,0.0f)*deltaTime;
			//angularForce += angularForce;
			rigidAMWriteMap[int3(groupId.x % 128, groupId.x / 128, 0)] += float4(angularForce, 0.0f) * deltaTime;
			locked = false;
			InterlockedCompareExchange(mutex, 1, 0, original);
		}
	}
}