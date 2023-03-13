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
	* ��ü�� 125���� particle���� ���� �� �ְ�,
	* particle���� 3���� position��(x,y,z), 3���� velocity��, 3���� �����߽ɿ� ���� ������� ��ġ���� �ִ�. 
	* �׸��� �ϳ��� thread���� �ִ� 5���� particle * 9���� position + velocty + position(COM)�� �����Ѵ�.
	* object ���� 125*9�� ũ��� align�Ѵ�. ���� x,y��ġ ���� 5(�ִ� z�� ������ particle����)�� ũ��� align�Ѵ�.
	* �̷��� align�� �ϴ� ������ ����� ũ�⸦ align���� �ʰ� ������ ��ܵθ� uav barrier�� �����ؼ� ����ȭ���Ѿ��ϱ� �����̴�.
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
			* 512�� particle Map�� x���� ũ���̴�.
			* �Ʒ����� ����ϴ� ���� center of mass�� ���� ������� particle���� ��ġ�̴�.
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
			* particle���� ��ġ ���
			* rigid body�� position�� quaternion�� �̿��Ѵ�.
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