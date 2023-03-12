/*
* depth Map�� �̿��ؼ� particle�� ������ particleMap�� ���ε��Ѵ�.
*/

Texture2DArray depthMap : register(t0);

RWTexture2DArray<float> particleMap : register(u0);

RWTexture2DArray<float> rigidBodyMap : register(u1);

RWTexture2D<int> rigidInfoMap : register(u2);

cbuffer constantBuffer : register(b0)
{
	int objIndex;
}

[numthreads(5,5,1)]
void CS(int id : SV_GroupIndex)
{
	/*
	* ��ü�� 125���� particle���� ���� �� �ְ�,
	* particle���� 3���� position��(x,y,z)�� �ִ�. 
	* �׸��� �ϳ��� thread���� �ִ� 5���� particle * 3���� position�� �����Ѵ�.
	* object ���� 125*3�� ũ��� align�Ѵ�. ���� x,y��ġ ���� 5(�ִ� z�� ������ particle����)�� ũ��� align�Ѵ�.
	* �̷��� align�� �ϴ� ������ ����� ũ�⸦ align���� �ʰ� ������ ��ܵθ� uav barrier�� �����ؼ� ����ȭ���Ѿ��ϱ� �����̴�.
	*/
	int particleIdx = objIndex * 3 * 125 + id * 5 * 3;
	int idx = id % 5;
	int idy = id / 5;
	int particleIdxX;
	int particleIdxY;

	int particleNumber = 0;
	for (int i = 0; i < 2; ++i)
	{
		float enter = depthMap.Load(int4(idx, idy, i*2, 0));
		float exit = depthMap.Load(int4(idx, idy, i*2+1, 0));

		//length += (exit - enter)*5.0f;
		for (float j = enter; j <= exit; j += 1.0f)
		{
			/*
			* 100�� particle Map�� x���� ũ���̴�.
			*/
			particleIdxX = particleIdx % 100;
			particleIdxY = particleIdx / 100;
			particleMap[int3(particleIdxX,particleIdxY,0)] = ((float)idx / 5.0f) * 2.0f - 0.8f;
			particleIdx += 1;

			particleIdxX = particleIdx % 100;
			particleIdxY = particleIdx / 100;
			particleMap[int3(particleIdxX, particleIdxY, 0)] = ((float)idy / 5.0f) * 2.0f - 0.8f;
			particleIdx += 1;

			particleIdxX = particleIdx % 100;
			particleIdxY = particleIdx / 100;
			particleMap[int3(particleIdxX, particleIdxY, 0)] = (j / 5.0f) * 2.0f - 1.0f;
			particleIdx += 1;
		}
		particleNumber += int((exit - enter)*5.0f);
	}
	InterlockedAdd(rigidInfoMap[int2(objIndex%256, objIndex/256)], particleNumber);
	GroupMemoryBarrierWithGroupSync();
}