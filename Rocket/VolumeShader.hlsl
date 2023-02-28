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

bool SphereIntersect(in float3 rayOrigin,in float3 rayDir,out float tMin,out float tMax)
{
	float3 center = float3(0.0f, 0.0f, 0.0f);
	float radius = 3.0f;

	float3 rayOriginToCenter = center - rayOrigin;
	float distance = length(cross(rayOriginToCenter, rayDir));

	tMin = 3.0f;
	tMax = 3.0f;

	if (distance > radius)
		return false;

	return true;
}

VertexOut VS( uint vertexId : SV_VertexID )
{
	VertexOut vout;

	vout.color = float4(0.5f, 0.5f, 0.5f, 1.0f);
	vout.pos = float4(coord[vertexId], 0.0f, 1.0f);
	vout.posW = mul(vout.pos, transpose(InvViewProjection));

	return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
	float tMin;
	float tMax;
	float3 dir = normalize(pin.posW.xyz - cameraPosition.xyz);

	if (!SphereIntersect(cameraPosition.xyz, dir, tMin, tMax))
		clip(-1);

	return pin.color;
}