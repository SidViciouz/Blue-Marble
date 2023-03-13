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

float3 RotationWithQuaternion(in float QX, in float QY, in float QZ, in float QW, in float3 p)
{
	float3x3 R = {
		1.0f - 2.0f * QY * QY - 2.0f * QZ * QZ, 2.0f * QX * QY + 2.0f * QZ * QW, 2.0f * QX * QZ - 2.0f * QY * QW,
		2.0f * QX * QY - 2.0f * QZ * QW, 1.0f - 2.0f * QX * QX - 2.0f * QZ * QZ, 2.0f * QY * QZ + 2.0f * QX * QW,
		2.0f * QX * QZ + 2.0f * QY * QW, 2.0f * QY * QX - 2.0f * QX * QW, 1.0f - 2.0f * QX * QX - 2.0f * QY * QY
	};

	return mul(p, R);
}

[numthreads(5,5,1)]
void CS(int id : SV_GroupIndex)
{
	/*
	* 물체당 125개의 particle까지 있을 수 있고,
	* particle마다 3개의 position값(x,y,z), 3개의 velocity값, 3개의 무게중심에 대한 상대적인 위치값이 있다. 
	* 그리고 하나의 thread마다 최대 5개의 particle * 9개의 position + velocty + position(COM)을 저장한다.
	* object 마다 125*9의 크기로 align한다. 또한 x,y위치 마다 5(최대 z축 방향의 particle개수)의 크기로 align한다.
	* 이렇게 align을 하는 이유는 저장될 크기를 align하지 않고 앞으로 당겨두면 uav barrier를 설정해서 동기화시켜야하기 때문이다.
	*/
	int particleIdx = objIndex * 9 * 125 + id * 5 * 9;
	int rigidIdx = objIndex * 13;
	int idx = id % 5;
	int idy = id / 5;
	int particleIdxX;
	int particleIdxY;
	float xFromCOM;
	float yFromCOM;
	float zFromCOM;

	int rigidIdxX = rigidIdx % 256;
	int rigidIdxY = rigidIdx / 256;
	float rigidPosX = rigidBodyMap[int3(rigidIdxX,rigidIdxY,0)];

	rigidIdx += 1;
	rigidIdxX = rigidIdx % 256;
	rigidIdxY = rigidIdx / 256;
	float rigidPosY = rigidBodyMap[int3(rigidIdxX, rigidIdxY, 0)];

	rigidIdx += 1;
	rigidIdxX = rigidIdx % 256;
	rigidIdxY = rigidIdx / 256;
	float rigidPosZ = rigidBodyMap[int3(rigidIdxX, rigidIdxY, 0)];

	rigidIdx += 1;
	rigidIdxX = rigidIdx % 256;
	rigidIdxY = rigidIdx / 256;
	float rigidQtX = rigidBodyMap[int3(rigidIdxX, rigidIdxY, 0)];

	rigidIdx += 1;
	rigidIdxX = rigidIdx % 256;
	rigidIdxY = rigidIdx / 256;
	float rigidQtY = rigidBodyMap[int3(rigidIdxX, rigidIdxY, 0)];

	rigidIdx += 1;
	rigidIdxX = rigidIdx % 256;
	rigidIdxY = rigidIdx / 256;
	float rigidQtZ = rigidBodyMap[int3(rigidIdxX, rigidIdxY, 0)];

	rigidIdx += 1;
	rigidIdxX = rigidIdx % 256;
	rigidIdxY = rigidIdx / 256;
	float rigidQtW = rigidBodyMap[int3(rigidIdxX, rigidIdxY, 0)];

	int particleNumber = 0;
	for (int i = 0; i < 2; ++i)
	{
		float enter = depthMap.Load(int4(idx, idy, i*2, 0));
		float exit = depthMap.Load(int4(idx, idy, i*2+1, 0));

		for (float j = enter; j <= exit && abs(exit-j) > 0.00001f; j += 0.2f)
		{
			/*
			* 512은 particle Map의 x방향 크기이다.
			* 아래에서 계산하는 값은 center of mass에 대한 상대적인 particle들의 위치이다.
			*/
			particleIdxX = particleIdx % 512;
			particleIdxY = particleIdx / 512;
			xFromCOM = ((float)idx / 5.0f) * 2.0f - 0.8f;
			particleMap[int3(particleIdxX, particleIdxY, 0)] = xFromCOM;
			particleIdx += 1;

			particleIdxX = particleIdx % 512;
			particleIdxY = particleIdx / 512;
			yFromCOM = ((float)idy / 5.0f) * 2.0f - 0.8f;
			particleMap[int3(particleIdxX, particleIdxY, 0)] = yFromCOM;
			particleIdx += 1;

			particleIdxX = particleIdx % 512;
			particleIdxY = particleIdx / 512;
			zFromCOM = j * 2.0f - 1.0f;
			particleMap[int3(particleIdxX, particleIdxY, 0)] = zFromCOM;
			particleIdx += 1;

			/*
			* particle들의 위치 계산
			* rigid body의 position과 quaternion을 이용한다.
			*/
			float3 rotatedP = RotationWithQuaternion(rigidQtX, rigidQtY, rigidQtZ, rigidQtW, float3(xFromCOM, yFromCOM, zFromCOM));

			particleIdxX = particleIdx % 512;
			particleIdxY = particleIdx / 512;
			particleMap[int3(particleIdxX, particleIdxY, 0)] = rotatedP.x + rigidPosX;
			particleIdx += 1;

			particleIdxX = particleIdx % 512;
			particleIdxY = particleIdx / 512;
			particleMap[int3(particleIdxX, particleIdxY, 0)] = rotatedP.y + rigidPosY;
			particleIdx += 1;

			particleIdxX = particleIdx % 512;
			particleIdxY = particleIdx / 512;
			particleMap[int3(particleIdxX, particleIdxY, 0)] = rotatedP.z + rigidPosZ;
			particleIdx += 1;
		}
		particleNumber += int((exit - enter)*5.0f);
	}
	InterlockedAdd(rigidInfoMap[int2(objIndex%256, objIndex/256)], particleNumber);
	GroupMemoryBarrierWithGroupSync();
}