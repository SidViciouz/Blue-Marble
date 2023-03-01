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
	float sigmaS = 0.5f;
	float sigmaT = 1.0f;
	float g = 0.25f;
	float stepSize = 0.1f;

	for (int i = 0; i < 3; ++i)
	{
		float tMin;
		float tMax;
		float3 lightToPos = position - lights[i].position;
		float3 posToCamera = normalize(cameraPosition - lights[i].position);
		float3 posToLight = -normalize(lightToPos);
		//point light로 가정
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
	float sigmaT = 1.0f;
	float att = 1.0f;
	float3 result = 0.0f;

	for (int i = 0; i < steps; ++i)
	{
		float3 ray = origin + (i*stepSize + tMin)*dir;
		att *= exp(-stepSize * sigmaT);
		result += sphereLi(ray) * att;
	}

	pin.color.w = att;
	pin.color.xyz += result.xyz;

	return pin.color;
}