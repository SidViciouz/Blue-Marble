struct Light
{
	float3 position;
	int type;
	float3 direction;
	int pad1;
	float3 color;
	int pad2;
};

struct VertexOut
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL;
	float2 tex : TEXTURE;
};

struct VertexIn
{
	float3 pos : POSITION;
	float3 normal : NORMAL;
	float2 tex : TEXTURE;
};

cbuffer obj : register(b0)
{
	float4x4 world;
}

cbuffer trans : register(b1)
{
	float4x4 viewProjection;
	Light lights[3];
}

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	float4 posW = mul(float4(vin.pos,1.0f), transpose(mul(world,viewProjection)));

	//homogeneous clip space�� ps�� ��������.(SV_POSITION semantic���� ������ ��)
	//vout.pos = mul(posW, viewProjection);
	vout.pos = posW;

	//non uniform scaling�̶�� ���������� world matrix�� ���Ѵ�. �׷��� ���� ��쿡�� inverse-transpose�� �̿��ؾ���.
	//�� ��꿡 �̿��� ���̹Ƿ� world space�� ��ȯ
	vout.normal = mul(vin.normal, (float3x3)world);
	
	//uv��ǥ��
	vout.tex = vin.tex;

	return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
	return float4(lights[2].color,1.0f);
	return float4(0.5f,0.5f,0.5f,1.0f);
}