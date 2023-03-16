RWTexture2DArray<float4> particlePosMap : register(u1);

RWTexture2DArray<float4> particleVelMap : register(u2);

RWTexture3D<int4> Grid : register(u8);

RWTexture2D<float4> ForceMap : register(u10);

float3 Repulsive(in int particleIdx,in int nParticleIdx)
{
	float3 force = 0.0f;
	float springC = 3.0f;
	float diameter = 1.0f;
	float3 particlePos = particlePosMap.Load(int4(particleIdx % 128, particleIdx / 128, 0, 0)).xyz;
	float3 nParticlePos = particlePosMap.Load(int4(nParticleIdx % 128, nParticleIdx / 128, 0, 0)).xyz;
	float distance = length(nParticlePos - particlePos);
	if (distance > diameter)
		return float3(0.0f, 0.0f, 0.0f);

	force = -springC*(diameter - distance) * normalize(nParticlePos - particlePos);
	return force;
}
float3 Damping(in int particleIdx, in int nParticleIdx)
{
	float3 force = 0.0f;
	float dampingC = 0.5;
	float3 particleVel = particleVelMap.Load(int4(particleIdx % 128, particleIdx / 128, 0, 0)).xyz;
	float3 nParticleVel = particleVelMap.Load(int4(nParticleIdx % 128, nParticleIdx / 128, 0, 0)).xyz;
	float3 relativeVel = nParticleVel - particleVel;

	force = dampingC * relativeVel;
	return force;
}
float3 Shear(in int particleIdx, in int nParticleIdx)
{
	return float3(0.0f, 0.0f, 0.0f);
}

float3 CalculateForce(in int particleIdx,in int3 position)
{
	float3 force = 0.0f;

	for (int i = -1; i < 2; ++i)
	{
		for (int j = -1; j < 2; ++j)
		{
			for (int k = -1; k < 2; ++k)
			{
				int3 neighbor = position + int3(i, j, k);

				if (neighbor.x < 0 || neighbor.x > 31 || neighbor.y < 0 || neighbor.y > 31 || neighbor.z < 0 || neighbor.z > 31)
					continue;

				int4 neighborIndices = Grid.Load(neighbor);

				if (neighborIndices.x == 0)
					continue;

				int nParticleIdx[4] = {
					neighborIndices.x,
					neighborIndices.y,
					neighborIndices.z,
					neighborIndices.w
				};

				/*
				* particle index를 0번부터 시작하도록 수정해야한다.
				* 그러지않으면, 0번 particle과 particle이 없는 것을 구분할 수 없다.
				*/
				for (int l = 0; l < 4; ++l)
				{
					if (nParticleIdx[l] == 0)
						break;
					if (particleIdx == nParticleIdx[l])
						continue;
					
					float diameter = 1.0f;
					float3 particlePos = particlePosMap.Load(int4(particleIdx % 128, particleIdx / 128, 0, 0)).xyz;
					float3 nParticlePos = particlePosMap.Load(int4(nParticleIdx[l] % 128, nParticleIdx[l] / 128, 0, 0)).xyz;
					float distance = length(nParticlePos - particlePos);
					if (distance > diameter)
						continue;
						
					force += Repulsive(particleIdx, nParticleIdx[l]) +Damping(particleIdx, nParticleIdx[l]) + Shear(particleIdx, nParticleIdx[l]);
				}

			}
		}
	}

	return force;
}

/*
* groupId는 grid에서의 z좌표, id는 grid에서는 x,y 좌표이다.
*/
[numthreads(32, 32, 1)]
void CS( uint3 groupId : SV_GroupID, uint3 id : SV_GroupThreadID)
{
	int4 gridValue = Grid.Load(int3(id.xy, groupId.x));
	
	if (gridValue.x == 0)
		return;

	int particle[4] =
	{
		gridValue.x,
		gridValue.y,
		gridValue.z,
		gridValue.w
	};

	for (int i = 0; i < 4; ++i)
	{
		int3 position = int3(id.xz, groupId.x);
		float3 force = CalculateForce(particle[i],position);
		ForceMap[int2(particle[i] % 128, particle[i] / 128)] = float4(force,0.0f);
	}
}