Texture2DArray depthMap : register(t0);

RWTexture2DArray<float4> particleCOMMap : register(u0);

/*
* particle ����, rigidbody�� offset
*/
RWTexture2D<int> mRigidInfos : register(u7);

RWTexture2D<float4> mRigidInertia : register(u9);


cbuffer constant : register(b0)
{
	int objIndex;
}

groupshared int mutex = 0;
groupshared int index = 0;

[numthreads(10, 10, 1)]
void CS( uint3 id : SV_DispatchThreadID )
{
	//���� rigidbody�� ���� particle���� offset
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

	//rigidbody�� �̷�� particle�� ����
	infoIdx -= 1;
	for (int i = 0; i < 2; ++i)
	{
		float enter = depthMap.Load(int4(id.x, id.y, i * 2, 0));
		float exit = depthMap.Load(int4(id.x, id.y, i * 2 + 1, 0));

		if (abs(exit - enter) < 0.00001f)
			continue;
		/*
		* depth�� 0.0f ~ 1.0f���� �̴�. ���� 5����ϸ� 0.2f�̴�.
		*/
		int enterGrid = (int)enter/0.1f;
		int exitGrid = (int)exit/0.1f;
		/*
		* j <= exitGrid�� �ƴ϶� j < exitGrid�� ��쿡�� �ػ󵵰� ������ partcicle�� �������� �ʰ� �н��ع�����.
		*/
		for (int j = enterGrid; j <= exitGrid; j++)
		{
			/*
			* [0,5]*[0,5]*[0,1] -> [-3,3]*[-3,3]*[-3,3]
			*/
			float4 coord = float4((float)id.x/10.0f*6.0f-3.0f, (float)id.y / 10.0f * 6.0f - 3.0f, (float)j/10.0f*6.0f-3.0f, 0.0f);

			bool locked = true;
			int original = 1;
			while (locked)
			{
				InterlockedCompareExchange(mutex, 0, 1, original);
				if (original == 0)
				{
					int idx = index + offset;
					/*
					* rigid body�� inertia_0 �� ����� ����.
					*/
					int inertiaIdx = objIndex * 6;
					mRigidInertia[int2(inertiaIdx % 128, inertiaIdx / 128)] +=
						float4(coord.y * coord.y + coord.z * coord.z, -coord.x * coord.y, -coord.x * coord.z, 0.0f);
					inertiaIdx += 1;
					mRigidInertia[int2(inertiaIdx % 128, inertiaIdx / 128)] +=
						float4(-coord.y * coord.x, coord.x * coord.x + coord.z * coord.z, -coord.y * coord.z, 0.0f);
					inertiaIdx += 1;
					mRigidInertia[int2(inertiaIdx % 128, inertiaIdx / 128)] +=
						float4(-coord.z * coord.x, -coord.z * coord.y, coord.x * coord.x + coord.y * coord.y, 0.0f);

					particleCOMMap[int3(idx % 128, idx / 128, 0)] = coord;
					//index + 1�� particle�� �����̴�. (offset�� 0���� �����ϹǷ�)
					mRigidInfos[int2(infoIdx % 128, infoIdx / 128)] = index + 1;
					index = index + 1;
					InterlockedCompareExchange(mutex, 1, 0, original);
					locked = false;
				}
			}
		}
		
		/*
		for (float j = enter; j <= exit && abs(exit - j) > 0.00001f; j += 0.2f)
		{
			bool locked = true;
			int original = 1;
			// x in [-2,2], y in [-2,2], z in [-2,2]
			float4 coord = float4((float)id.x - 2.0f, (float)id.y - 2.0f, j*4.0f-2.0f,0.0f);
			
			while (locked)
			{
				InterlockedCompareExchange(mutex, 0, 1, original);
				if (original == 0)
				{
					int idx = index + offset;

					
					// rigid body�� inertia_0 �� ����� ����.
					
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
					//index + 1�� particle�� �����̴�. (offset�� 0���� �����ϹǷ�)
					mRigidInfos[int2(infoIdx % 128, infoIdx / 128)] = index + 1;
					index = index + 1;
					InterlockedCompareExchange(mutex, 1, 0, original);
					locked = false;
				}
			}
		}*/
	}
}