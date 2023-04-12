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
	float4x4 lightView;
	float4x4 lightProjection;
};

cbuffer obj : register(b0)
{
	float4x4 world;
	float3 diffuseAlbedo;
	float roughness;
	float3 fresnel;
	int pad;
}

cbuffer env : register(b1)
{
	float4x4 view;
	float4x4 projection;
	Light lights[3];
	float3 cameraPosition;
	int pad1;
	float3 cameraFront;
	int pad2;
	float4x4 InvViewProjection;
	float currentTime;
	int pad3;
	int pad4;
	int pad5;
}

Texture1D<float4> gradients : register(t0);
Texture1D<int> permutation : register(t1);


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

float smoothstep(float t)
{
	return t * t * (3 - 2 * t);
}

int hash(int x, int y, int z)
{
	return permutation[permutation[permutation[x] + y] + z];
}

float evalDensity(float3 position)
{
	int x0 = (int)floor(position.x) & 127;
	int y0 = (int)floor(position.y) & 127;
	int z0 = (int)floor(position.z) & 127;

	int x1 = (x0 + 1) & 127;
	int y1 = (y0 + 1) & 127;
	int z1 = (z0 + 1) & 127;

	float tx = position.x - (int)floor(position.x);
	float ty = position.y - (int)floor(position.y);
	float tz = position.z - (int)floor(position.z);

	float u = smoothstep(tx);
	float v = smoothstep(ty);
	float w = smoothstep(tz);

	float3 c000 = gradients[hash(x0, y0, z0)].xyz;
	float3 c100 = gradients[hash(x1, y0, z0)].xyz;
	float3 c010 = gradients[hash(x0, y1, z0)].xyz;
	float3 c110 = gradients[hash(x1, y1, z0)].xyz;
	float3 c001 = gradients[hash(x0, y0, z1)].xyz;
	float3 c101 = gradients[hash(x1, y0, z1)].xyz;
	float3 c011 = gradients[hash(x0, y1, z1)].xyz;
	float3 c111 = gradients[hash(x1, y1, z1)].xyz;

	float u0 = tx;
	float u1 = tx - 1;
	float v0 = ty;
	float v1 = ty - 1;
	float w0 = tz;
	float w1 = tz - 1;

	float3 p000 = float3(u0, v0, w0);
	float3 p100 = float3(u1, v0, w0);
	float3 p010 = float3(u0, v1, w0);
	float3 p110 = float3(u1, v1, w0);
	float3 p001 = float3(u0, v0, w1);
	float3 p101 = float3(u1, v0, w1);
	float3 p011 = float3(u0, v1, w1);
	float3 p111 = float3(u1, v1, w1);

	float a = lerp(dot(c000, p000), dot(c100, p100), u);
	float b = lerp(dot(c010, p010), dot(c110, p110), u);
	float c = lerp(dot(c001, p001), dot(c101, p101), u);
	float d = lerp(dot(c011, p011), dot(c111, p111), u);

	float e = lerp(a, b, v);
	float f = lerp(c, d, v);

	return lerp(e, f, w);
}

float noise(float3 position)
{
	float noiseSum = 0.0f;
	float amplitude = 1.0f;
	float frequency = 1.0f;
	int layers = 7;

	position.x += currentTime*10.0f;

	for (int i = 0; i < layers; ++i)
	{
		noiseSum += evalDensity(position * frequency) * amplitude;
		amplitude *= 2.0f;
		frequency *= 0.5f;
	}

	return noiseSum/(float)layers;
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
	float stepSize = 0.5f;
	float density = 1.0f;
	int steps = 10;

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
		//int steps = (length(lightToPos) - tMin) / stepSize;
		float result = 0.0f;
		float stepSize = (length(lightToPos) - tMin) / (float)steps;

		for (int j = 0; j < steps; ++j)
		{
			float3 coord = 10.0f*( x + posToLight * (tMin + stepSize * j));
			coord -= float3(40, -50, -50);
			density = noise(coord);
			att *= exp(-stepSize * sigmaT * density);
		}

		result = att / (fourPi * pow(1 + g * g - 2 * g * dot(posToCamera, posToLight), 1.5f)) * (1 - g * g);

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
	//float stepSize = 0.5f;
	float sigmaT = 0.2f;
	float sigmaS = 0.1f;
	float att = 1.0f;
	float3 result = 0.0f;
	float density = 1.0f;
	int steps = 10;


	if (CubeIntersect(rayOrigin, rayDir, tMin, tMax) == false)
		clip(-1);
	
	//float steps = (tMax - tMin) / stepSize;
	float stepSize = (tMax - tMin) / (float)steps;

	for (int i = 0; i < steps; ++i)
	{
		float3 coord =10.0f*( rayOrigin + rayDir * (tMin+ stepSize*i));
		coord -= float3(40, -40, -50);
		density = noise(coord);
		att *= exp(-sigmaT * stepSize * density);
		if(density > 1.0f)
			result += pf(rayOrigin + rayDir*(tMin+ i*stepSize)) * sigmaS * att;
	}

	return float4(result,att);
}