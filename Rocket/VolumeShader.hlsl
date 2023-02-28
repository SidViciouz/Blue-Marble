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

float3 sphereLi(float3 position,Light light)
{
	float tMin;
	float tMax;
	float lightToPos = position - light.position;
	//point light로 가정
	SphereIntersect(light.position, normalize(lightToPos), tMin, tMax);

	float stepSize = 0.1f;
	int steps = (length(lightToPos) - tMin)/stepSize;
	float sigmaT = 0.2f;
	float result = 0.0f;
	float att = 1.0f;

	//result = exp((tMin - length(lightToPos)) * sigmaT);

	
	for (int i = 0; i < steps; ++i)
	{
		att *= exp(-stepSize * sigmaT);
	}
	result += att;
	

	return light.color * result;
}

VertexOut VS( uint vertexId : SV_VertexID )
{
	VertexOut vout;

	vout.color = float4(0.1f, 0.1f, 0.1f, 1.0f);
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

	for (int i = 0; i < steps; ++i)
	{
		float3 ray = origin + (i*stepSize + tMin)*dir;
		att *= exp(-stepSize * sigmaT);
		result += sphereLi(ray,lights[0]) * att * stepSize;
		
		//pin.color.w -= 0.01f;
	}
	pin.color.w -= result.x;
	pin.color.xyz += result.xyz;

	return pin.color;
}