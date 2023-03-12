/*
* depth Map을 이용해서 particle을 생성해 particleMap에 업로드한다.
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
	* 물체당 125개의 particle까지 있을 수 있고,
	* particle마다 3개의 position값(x,y,z)이 있다. 
	* 그리고 하나의 thread마다 최대 5개의 particle * 3개의 position을 저장한다.
	* object 마다 125*3의 크기로 align한다. 또한 x,y위치 마다 5(최대 z축 방향의 particle개수)의 크기로 align한다.
	* 이렇게 align을 하는 이유는 저장될 크기를 align하지 않고 앞으로 당겨두면 uav barrier를 설정해서 동기화시켜야하기 때문이다.
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
			* 100은 particle Map의 x방향 크기이다.
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