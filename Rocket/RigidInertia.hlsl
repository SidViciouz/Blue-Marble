RWTexture2DArray<float4> particleCOMMap : register(u0);

RWTexture2DArray<float4> rigidQuatMap : register(u4);

RWTexture2D<int> mRigidInfos : register(u7);

RWTexture2D<float4> rigidInertia : register(u9);

cbuffer constant : register(b0)
{
	int objNum;
}

float3x3 InverseMatrix(float3x3 m)
{

	float det = determinant(m);

	float3x3 tM = transpose(m);

	float xx = determinant(float2x2(tM._m11, tM._m12,
		tM._m21, tM._m22));
	float xy = determinant(float2x2(tM._m10, tM._m12,
		tM._m20, tM._m22));
	float xz = determinant(float2x2(tM._m10, tM._m11,
		tM._m20, tM._m21));

	float yx = determinant(float2x2(tM._m01, tM._m02,
		tM._m21, tM._m22));
	float yy = determinant(float2x2(tM._m00, tM._m02,
		tM._m20, tM._m22));
	float yz = determinant(float2x2(tM._m00, tM._m01,
		tM._m20, tM._m21));

	float zx = determinant(float2x2(tM._m01, tM._m02,
		tM._m11, tM._m12));
	float zy = determinant(float2x2(tM._m00, tM._m02,
		tM._m10, tM._m12));
	float zz = determinant(float2x2(tM._m00, tM._m01,
		tM._m10, tM._m11));

	float3x3 inv =
		float3x3(
			xx,-xy,xz,
			-yx,yy,-yz,
			zx,-zy,zz
			);

	inv = inv / det;

	return inv;
}

float3x3 GetRotationMatrixFromQuaternion(float4 Q)
{
	float3x3 R = {
		1.0f - 2.0f * Q.y * Q.y - 2.0f * Q.z * Q.z, 2.0f * Q.x * Q.y + 2.0f * Q.z * Q.w, 2.0f * Q.x * Q.z - 2.0f * Q.y * Q.w,
		2.0f * Q.x * Q.y - 2.0f * Q.z * Q.w, 1.0f - 2.0f * Q.x * Q.x - 2.0f * Q.z * Q.z, 2.0f * Q.y * Q.z + 2.0f * Q.x * Q.w,
		2.0f * Q.x * Q.z + 2.0f * Q.y * Q.w, 2.0f * Q.y * Q.x - 2.0f * Q.x * Q.w, 1.0f - 2.0f * Q.x * Q.x - 2.0f * Q.y * Q.y
	};

	return R;
}

[numthreads(1024, 1, 1)]
void CS(int id : SV_GroupIndex)
{

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


	int inertiaIdx = rigidBodyIndex * 6;
	float3 inertia0 = rigidInertia.Load(int3(inertiaIdx % 128, inertiaIdx / 128, 0)).xyz;
	float3 inertia1 = rigidInertia.Load(int3((inertiaIdx + 1) % 128, (inertiaIdx + 1) / 128, 0)).xyz;
	float3 inertia2 = rigidInertia.Load(int3((inertiaIdx + 2) % 128, (inertiaIdx + 2) / 128, 0)).xyz;
	float3x3 inertiaMatrix = float3x3(
		inertia0.x,inertia0.y,inertia0.z,
		inertia1.x, inertia1.y, inertia1.z,
		inertia2.x, inertia2.y, inertia2.z
		);

	float3x3 rotationMatrix = GetRotationMatrixFromQuaternion(rigidQuatMap.Load(int4(rigidBodyIndex % 128, rigidBodyIndex / 128, 0, 0)));
	float3x3 rotatedInverseInertia = rotationMatrix * InverseMatrix(inertiaMatrix) * transpose(rotationMatrix);

	inertiaIdx += 3;
	rigidInertia[int2(inertiaIdx % 128, inertiaIdx / 128)] = float4(rotatedInverseInertia._m00, rotatedInverseInertia._m01, rotatedInverseInertia._m02,0.0f);
	inertiaIdx += 1;
	rigidInertia[int2(inertiaIdx % 128, inertiaIdx / 128)] = float4(rotatedInverseInertia._m10, rotatedInverseInertia._m11, rotatedInverseInertia._m12, 0.0f);
	inertiaIdx += 1;
	rigidInertia[int2(inertiaIdx % 128, inertiaIdx / 128)] = float4(rotatedInverseInertia._m20, rotatedInverseInertia._m21, rotatedInverseInertia._m22, 0.0f);
}

