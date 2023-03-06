struct VertexOut
{
	float4 pos : SV_POSITION;
	float4 posW : POSITION;
};

struct Coord
{
	float x;
	float y;
	float z;
};

struct Light
{
	float3 position;
	int type;
	float3 direction;
	int pad1;
	float3 color;
	int pad2;
};

cbuffer obj : register(b0)
{
	float4x4 world;
	float3 diffuseAlbedo;
	float roughness;
	float3 fresnel;
	int pad;
}

cbuffer trans : register(b1)
{
	float4x4 view;
	float4x4 projection;
	Light lights[3];
	float3 cameraPosition;
	int pad1;
	float3 cameraFront;
	int pad2;
	float4x4 InvViewProjection;
}

RWTexture3D<int> textureMap : register(u0);


static Coord coord[36] = {
	{1.0f,1.0f,-1.0f},
	{-1.0f,1.0f,1.0f},
	{1.0f,1.0f,1.0f},

	{1.0f,1.0f,-1.0f},
	{-1.0f,1.0f,-1.0f},
	{-1.0f,1.0f,1.0f},

	{1.0f,1.0f,-1.0f},
	{1.0f,1.0f,1.0f},
	{1.0f,-1.0f,-1.0f},

	{1.0f,-1.0f,-1.0f},
	{1.0f,1.0f,1.0f},
	{1.0f,-1.0f,1.0f},

	{-1.0f,1.0f,-1.0f},
	{1.0f,1.0f,-1.0f},
	{1.0f,-1.0f,-1.0f},

	{-1.0f,-1.0f,-1.0f},
	{-1.0f,1.0f,-1.0f},
	{1.0f,-1.0f,-1.0f},

	{-1.0f,1.0f,1.0f},
	{-1.0f,1.0f,-1.0f},
	{-1.0f,-1.0f,-1.0f},

	{-1.0f,1.0f,1.0f},
	{-1.0f,-1.0f,-1.0f},
	{-1.0f,-1.0f,1.0f},

	{-1.0f,-1.0f,-1.0f},
	{1.0f,-1.0f,-1.0f},
	{1.0f,-1.0f,1.0f},

	{-1.0f,-1.0f,-1.0f},
	{1.0f,-1.0f,1.0f},
	{-1.0f,-1.0f,1.0f},

	{1.0f,1.0f,1.0f},
	{-1.0f,1.0f,1.0f},
	{1.0f,-1.0f,1.0f},

	{-1.0f,1.0f,1.0f},
	{-1.0f,-1.0f,1.0f},
	{1.0f,-1.0f,1.0f}
};

static float3 cubeNormals[6] =
{
	{1.0f,0.0f,0.0f},
	{-1.0f,0.0f,0.0f},
	{0.0f,1.0f,0.0f},
	{0.0f,-1.0f,0.0f},
	{0.0f,0.0f,1.0f},
	{0.0f,0.0f,-1.0f}
};

VertexOut VS( uint vertexId : SV_VertexID)
{
	VertexOut vout;

	vout.posW = mul(float4(coord[vertexId % 36],1.0f), transpose(world));

	vout.pos = mul(mul(vout.posW, transpose(view)), transpose(projection));

	return vout;
}

bool planeIntersect(in int planeNumber,in float3 CubePosition,in float3 CubeScale,in float3 intersect)
{
	if (planeNumber == 0 || planeNumber == 1) // +X, -X
	{
		if ((CubePosition.y - CubeScale.y <= intersect.y) && (intersect.y <= CubePosition.y + CubeScale.y)
			&& (CubePosition.z - CubeScale.z <= intersect.z) && (intersect.z <= CubePosition.z + CubeScale.z))
			return true;
	}
	else if (planeNumber == 2 || planeNumber ==3) // +Y, -Y
	{
		if ((CubePosition.x - CubeScale.x <= intersect.x) && (intersect.x <= CubePosition.x + CubeScale.x)
			&& (CubePosition.z - CubeScale.z <= intersect.z) && (intersect.z <= CubePosition.z + CubeScale.z))
			return true;
	}
	else if(planeNumber == 4 || planeNumber == 5) // +Z, -Z
	{
		if ((CubePosition.y - CubeScale.y <= intersect.y) && (intersect.y <= CubePosition.y + CubeScale.y)
			&& (CubePosition.x - CubeScale.x <= intersect.x) && (intersect.x <= CubePosition.x + CubeScale.x))
			return true;
	}

	return false;
}

bool CubeIntersect(in float3 rayOrigin,in float3 rayDir,out float tMin,out float tMax)
{
	tMin = 100.0f;
	tMax = 0.0f;

	//hlsl 에서는  colume major이다.
	float3 CubePosition = float3(world[0][3], world[1][3], world[2][3]);
	float3 CubeScale = float3(world[0][0], world[1][1], world[2][2]);
	float3 CubeVertex[8] =
	{
		{CubePosition.x + CubeScale.x, CubePosition.y + CubeScale.y, CubePosition.z + CubeScale.z},
		{CubePosition.x + CubeScale.x, CubePosition.y + CubeScale.y, CubePosition.z - CubeScale.z},
		{CubePosition.x - CubeScale.x, CubePosition.y + CubeScale.y, CubePosition.z + CubeScale.z},
		{CubePosition.x - CubeScale.x, CubePosition.y + CubeScale.y, CubePosition.z - CubeScale.z},
		{CubePosition.x + CubeScale.x, CubePosition.y - CubeScale.y, CubePosition.z + CubeScale.z},
		{CubePosition.x + CubeScale.x, CubePosition.y - CubeScale.y, CubePosition.z - CubeScale.z},
		{CubePosition.x - CubeScale.x, CubePosition.y - CubeScale.y, CubePosition.z + CubeScale.z},
		{CubePosition.x - CubeScale.x, CubePosition.y - CubeScale.y, CubePosition.z - CubeScale.z}
	};
	float3 pointOnPlane[6] =
	{
		CubeVertex[0], // +X
		CubeVertex[7], // -X
		CubeVertex[0], // +Y
		CubeVertex[7], // -Y
		CubeVertex[0], // +Z
		CubeVertex[7]  // -Z
	};

	//box의 각 평면과의 교점이 존재하는지 계산하는 부분.
	for (int i = 0; i < 6; ++i)
	{
		if (dot(cubeNormals[i], rayDir) == 0) // 교점이 없거나 무한히 많다.
			continue;

		float t = (-dot(cubeNormals[i], rayOrigin) + dot(cubeNormals[i], pointOnPlane[i])) / dot(cubeNormals[i], rayDir);
		if (t < 0) // 교점이 없다.
			continue;

		float3 intersect = rayOrigin + t * rayDir;

		if (planeIntersect(i, CubePosition, CubeScale, intersect) == false)
			continue;

		tMin = min(tMin, t);
		tMax = max(tMax, t);
	}

	if (tMin == 100.0f && tMax == 0.0f)
		return false;
	else
		return true;
}

float3 pf(float3 x)
{

	float3 ret = 0.0f;
	float fourPi = 4 * 3.14159265f;
	float sigmaT = 0.2f;
	float g = 0.25f;
	float stepSize = 0.1f;
	float density = 1.0f;

	for (int i = 0; i < 3; ++i)
	{
		float tMin = 0.0f;
		float tMax = 100.0f;
		float3 lightToPos = x - lights[i].position;
		float3 posToCamera = normalize(cameraPosition - lights[i].position);
		float3 posToLight = -normalize(lightToPos);

		if (lights[i].type == 2 && dot(-posToLight, lights[i].direction) < cos(3.14f / 6.0f))
			continue;
		//point또는 spot light로 가정
		if (CubeIntersect(lights[i].position, normalize(lightToPos), tMin, tMax) == false)
			continue;

		float att = 1.0f;
		int steps = (length(lightToPos) - tMin) / stepSize;
		float result = 0.0f;

		for (int j = 0; j < steps; ++j)
		{
			density = textureMap.Load(int4(0, 0, 0, 0));
			att *= exp(-stepSize * sigmaT * density);
		}

		result += att / (fourPi * pow(1 + g * g - 2 * g * dot(posToCamera, posToLight), 1.5f)) * (1 - g * g);

		ret += lights[i].color * result;
	}

	return ret;

}

float4 PS(VertexOut pin) : SV_Target
{
	float3 rayOrigin = cameraPosition;
	float3 rayDir = normalize(pin.posW.xyz - cameraPosition);
	float tMin = 0.0f;
	float tMax = 100.0f;
	float stepSize = 0.1f;
	float sigmaT = 0.2f;
	float sigmaS = 0.1f;
	float att = 1.0f;
	float3 result = 0.0f;
	float density = 1.0f;


	if (CubeIntersect(rayOrigin, rayDir, tMin, tMax) == false)
		clip(-1);
	
	float steps = (tMax - tMin) / stepSize;

	for (int i = 0; i < steps; ++i)
	{
		density = textureMap.Load(int4(0, 0, 0, 0));
		att *= exp(-sigmaT * stepSize * density);
		result += pf(rayOrigin + rayDir*(tMin+ i*stepSize)) * sigmaS * att;
	}
	textureMap[int3(0, 0, 0)] += 0.1f;
	//InterlockedAdd(textureMap[int3(0, 0, 0)], 1);
	return float4(result,att);
}