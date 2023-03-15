RWTexture2DArray<float4> particleCOMMap : register(u0);

RWTexture2DArray<float4> particlePosMap : register(u1);

RWTexture2D<int> mRigidInfos : register(u7);

RWTexture3D<int4> Grid : register(u8);

cbuffer constant : register(b0)
{
	int objNum;
}

groupshared int mutex[16][16][16];

[numthreads(1024, 1, 1)]
void CS( int id : SV_GroupIndex)
{
	for(int k=0; k<16; ++k)
		mutex[id % 16][id / 16][k] = 0;

	GroupMemoryBarrierWithGroupSync();

	int rigidBodyIndex = -1;

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
			break;
		}
	}

	if (i >= objNum)
		return;

	float4 position = particlePosMap.Load(int4(particleIdxX, particleIdxY, 0, 0));
	//
	//position.x += 5.0f;
	//position.y += 5.0f;
	//position.z += 5.0f;
	//
	int3 integerPosition = int3((int)position.x, (int)position.y, (int)position.z);

	bool locked = true;
	while (locked)
	{
		int original = 1;
		InterlockedCompareExchange(mutex[integerPosition.x/2][integerPosition.y/2][integerPosition.z/2], 0, 1, original);
		if (original == 0)
		{
			int4 gridValue = Grid.Load(integerPosition);
			if (gridValue.x == 0)
				Grid[integerPosition] += int4(id, 0, 0, 0);
			else if (gridValue.y == 0)
				Grid[integerPosition] += int4(0, id,0,0);
			else if (gridValue.z == 0)
				Grid[integerPosition] += int4(0, 0, id, 0);
			else if (gridValue.w == 0)
				Grid[integerPosition] += int4(0, 0, 0, id);
			InterlockedCompareExchange(mutex[integerPosition.x/2][integerPosition.y/2][integerPosition.z/2], 1, 0, original);
			locked = false;
		}
	}
}