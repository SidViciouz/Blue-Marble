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
	float2 tex : TEXTURE;
	float3 normal : NORMAL;
};

struct VertexIn
{
	float3 pos : POSITION;
	float2 tex : TEXTURE;
	float3 normal : NORMAL;
};

cbuffer obj : register(b0)
{
	float4x4 world;
	float3 diffuseAlbedo;
	int pad1;
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
	vout.normal = mul(vin.normal,transpose((float3x3)world));
	
	//uv��ǥ��
	vout.tex = vin.tex;

	return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
	//normal vector ����ȭ�� �ؾ��ϳ�?
	float3 L = normalize(lights[0].position - pin.pos);
	float factor = max(dot(L, pin.normal),0.0f);

	return float4(factor*lights[0].color*diffuseAlbedo,1.0f);
}