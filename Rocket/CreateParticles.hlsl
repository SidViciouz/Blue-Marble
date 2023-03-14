Texture2DArray depthMap : register(t0);

RWTexture2DArray<float4> particleCOMMap : register(u0);

/*
* particle 개수, rigidbody의 offset
*/
RWTexture2D<int> mRigidInfos : register(u7);

RWTexture2D<float4> mRigidInertia : register(u9);


cbuffer constant : register(b0)
{
	int objIndex;
}

groupshared int mutex = 0;
groupshared int index = 0;

[numthreads(5, 5, 1)]
void CS( uint3 id : SV_DispatchThreadID )
{
	//같은 rigidbody에 대한 particle들의 offset
	int infoIdx = objIndex * 2 + 1;

	if (objIndex == 0)
	{
		InterlockedCompareStore(mRigidInfos[int2(infoIdx % 128, infoIdx / 128)], 0, 0);
	}
	else
	{
		int prevInfoIdx = (objIndex - 1) * 2;
		InterlockedCompareStore(mRigidInfos[int2(infoIdx % 128, infoIdx / 128)], 0, mRigidInfos[int2(prevInfoIdx % 128, prevInfoIdx / 128)]
			+ mRigidInfos[int2((prevInfoIdx + 1) % 128, (prevInfoIdx + 1) / 128)]);
	}

	GroupMemoryBarrierWithGroupSync();

	int offset = mRigidInfos[int2(infoIdx % 128, infoIdx / 128)];

	//rigidbody를 이루는 particle의 개수
	infoIdx -= 1;
	for (int i = 0; i < 2; ++i)
	{
		float enter = depthMap.Load(int4(id.x, id.y, i * 2, 0));
		float exit = depthMap.Load(int4(id.x, id.y, i * 2 + 1, 0));
		/*
		* depth는 0.0f ~ 1.0f까지 이다. 따라서 5등분하면 0.2f이다.
		*/
		for (float j = enter; j <= exit && abs(exit - j) > 0.00001f; j += 0.2f)
		{
			bool locked = true;
			int original = 1;
			float4 coord = float4((float)id.x / 4.0f, (float)id.y / 4.0f, j*2.0f-1.0f,0.0f);

			while (locked)
			{
				InterlockedCompareExchange(mutex, 0, 1, original);
				if (original == 0)
				{
					int idx = index + offset;

					/*
					* rigid body의 inertia_0 를 계산후 저장.
					*/
					int inertiaIdx = objIndex * 6;
					mRigidInertia[int2(inertiaIdx % 128, inertiaIdx / 128)] += 
						float4(coord.y*coord.y + coord.z*coord.z, -coord.x * coord.y, -coord.x * coord.z,0.0f);
					inertiaIdx += 1;
					mRigidInertia[int2(inertiaIdx % 128, inertiaIdx / 128)] += 
						float4(-coord.y * coord.x, coord.x * coord.x + coord.z * coord.z, -coord.y * coord.z,0.0f);
					inertiaIdx += 1;
					mRigidInertia[int2(inertiaIdx % 128, inertiaIdx / 128)] += 
						float4(-coord.z * coord.x, -coord.z * coord.y, coord.x * coord.x + coord.y * coord.y,0.0f);

					particleCOMMap[int3(idx % 128, idx / 128, 0)] = coord;
					//index + 1은 particle의 개수이다. (offset은 0부터 시작하므로)
					mRigidInfos[int2(infoIdx % 128, infoIdx / 128)] = index + 1;
					index = index + 1;
					InterlockedCompareExchange(mutex, 1, 0, original);
					locked = false;
				}
			}
		}
	}
}