struct VertexOut
{
	float4 pos : SV_POSITION;
	float4 color : COLOR;
	float4 posW : POSITION;
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
	float currentTime;
	int pad3;
	int pad4;
	int pad5;
}

Texture1D<float4> gradients : register(t0);
Texture1D<int> permutation : register(t1);

struct Coord
{
	float x;
	float y;
};


static Coord coord[6] = {
	{-1.0f,1.0f},
	{1.0f,1.0f},
	{1.0f,-1.0f},
	{-1.0f,1.0f},
	{1.0f,-1.0f},
	{-1.0f,-1.0f}
};


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

	position.x += currentTime * 10.0f;

	for (int i = 0; i < layers; ++i)
	{
		noiseSum += evalDensity(position * frequency) * amplitude;
		amplitude *= 2.0f;
		frequency *= 0.5f;
	}

	return noiseSum / (float)layers;
}


//rayDir는 normalize된 상태여야함.
bool SphereIntersect(in float3 rayOrigin,in float3 rayDir,out float tMin,out float tMax)
{
	float3 center = float3(0.0f, 0.0f, 0.0f);
	float radius = 3.0f;

	float3 rayOriginToCenter = center - rayOrigin;

	if (dot(rayOriginToCenter, rayDir) < 0 && length(rayOriginToCenter) > radius)
		return false;

	float distance = length(cross(rayOriginToCenter, rayDir));

	tMin = 0.0f;
	tMax = 0.0f;

	if (distance > radius)
		return false;

	float t = dot(rayOriginToCenter, rayDir);
	float dt = sqrt(radius * radius - distance * distance);

	tMin = t - dt;
	tMax = t + dt;

	return true;
}

float3 sphereLi(float3 position)
{
	float3 ret = float3(0.0f, 0.0f, 0.0f);
	float fourPi = 4 * 3.14159265f;
	float sigmaS = 0.1f;
	float sigmaT = 0.2f;
	float g = 0.25f;
	float stepSize = 0.1f;

	for (int i = 0; i < 3; ++i)
	{
		float tMin;
		float tMax;
		float3 lightToPos = position - lights[i].position;
		float3 posToCamera = normalize(cameraPosition - lights[i].position);
		float3 posToLight = -normalize(lightToPos);

		if (lights[i].type == 2 && dot(-posToLight, lights[i].direction) < cos(3.14f / 6.0f))
			continue;

		//point또는 spot light로 가정
		SphereIntersect(lights[i].position, normalize(lightToPos), tMin, tMax);

		float att = 1.0f;
		int steps = (length(lightToPos) - tMin) / stepSize;
		float result = 0.0f;

		for (int j = 0; j < steps; ++j)
			att *= exp(-stepSize * sigmaT);
		
		result += att / (fourPi * pow(1 + g * g - 2 * g * dot(posToCamera, posToLight), 1.5f)) * (1 - g * g);

		ret += lights[i].color * result * sigmaS;
	}
	return ret;
}

VertexOut VS( uint vertexId : SV_VertexID )
{
	VertexOut vout;

	vout.color = float4(0.0f, 0.0f, 0.0f, 1.0f);
	vout.pos = float4(coord[vertexId], 0.0f, 1.0f);
	vout.posW = mul(vout.pos, transpose(InvViewProjection));

	return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
	float tMin;
	float tMax;
	float3 dir = normalize(pin.posW.xyz - cameraPosition.xyz);
	float3 origin = cameraPosition.xyz;

	if (!SphereIntersect(origin, dir, tMin, tMax))
		clip(-1);

	float stepSize = 0.1f;
	int steps = (tMax - tMin) / stepSize;
	float sigmaT = 0.2f;
	float att = 1.0f;
	float3 result = 0.0f;
	float density = 1.0f;

	for (int i = 0; i < steps; ++i)
	{
		float3 ray = origin + (i*stepSize + tMin)*dir;
		density = noise(ray*100.0f);
		att *= exp(-stepSize * sigmaT * density);
		result += sphereLi(ray) * att;
	}

	pin.color.w = att;
	pin.color.xyz += result.xyz;

	return pin.color;
}