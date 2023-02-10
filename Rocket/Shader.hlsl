struct VertexOut
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL;
	float2 tex : TEXTURE;
};

struct VertexIn
{
	float4 pos : POSITION;
	float3 normal : NORMAL;
	float2 tex : TEXTURE;
};

cbuffer obj : register(b1)
{
	float4x4 world;
}

cbuffer trans : register(b2)
{
	float4x4 viewProjection;
}

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	float4 posW = mul(vin.pos, world);

	//homogeneous clip space�� ps�� ��������.(SV_POSITION semantic���� ������ ��)
	vout.pos = mul(posW, viewProjection);

	//non uniform scaling�̶�� ���������� world matrix�� ���Ѵ�. �׷��� ���� ��쿡�� inverse-transpose�� �̿��ؾ���.
	//�� ��꿡 �̿��� ���̹Ƿ� world space�� ��ȯ
	vout.normal = mul(vin.normal, (float3x3)world);
	
	//uv��ǥ��
	vout.tex = vin.tex;

	return vout;
}