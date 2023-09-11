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

struct VertexOut
{
	float4 pos : SV_POSITION;
	float2 tex : TEXTURE;
	float3 normal : NORMAL;
	float3 posW : POSITION;
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
}

texture2D<float4> textureMap : register(t0);

SamplerState textureSampler : register(s0);

VertexOut VS(VertexIn vin)
{
	VertexOut vout;

	float4 posW = mul(float4(vin.pos, 1.0f), transpose(world));

	float4 posH = mul(mul(posW, transpose(view)), transpose(projection));

	//homogeneous clip space�� ps�� ��������.(SV_POSITION semantic���� ������ ��)
	//vout.pos = mul(posW, viewProjection);
	vout.pos = posH;

	//�ݿ� ������ halfway vector�� ����ϱ� ���ؼ� �ʿ���.
	vout.posW = posW.xyz;

	//non uniform scaling�̶�� ���������� world matrix�� ���Ѵ�. �׷��� ���� ��쿡�� inverse-transpose�� �̿��ؾ���.
	//�� ��꿡 �̿��� ���̹Ƿ� world space�� ��ȯ
	vout.normal = mul(vin.normal, (float3x3)transpose(world));

	//uv��ǥ��
	vout.tex = vin.tex;

	return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
	float3 diffuse = diffuseAlbedo * textureMap.Sample(textureSampler, pin.tex);
	float3 L = {1.0f,1.0f,1.0f};
	float rambertTerm = 0.0f;
	float4 color = float4(diffuse *float3(0.1f,0.1f,0.1f), 1.0f);
	float3 fresnelTerm;
	float roughnessTerm;
	
	pin.normal = normalize(pin.normal);

	for (int i = 0; i < 3; ++i)
	{
		if (lights[i].type == 0) //directional light
		{
			L = -normalize(lights[i].direction);
		}
		else if (lights[i].type == 1) // point light
		{
			L = normalize(lights[i].position - pin.posW);
		}
		else //spot light
		{
			L = normalize(lights[i].position - pin.posW);

			if (dot(-L, lights[i].direction) < cos(3.14f/6.0f))
			{
				continue;
			}
		}

		//���� �Ի簢�� ���� ���������� ������ ���� ���� �޶����� ǥ��
		rambertTerm = max(dot(L, pin.normal), 0.0f);

		//���� �Ի簢�� ���� �ݻ�Ǵ� ���� ���� �޶����� ���� ǥ��(�ݿ� ������ ���)
		float cosTerm = 1.0f - saturate(dot(pin.normal, L));
		fresnelTerm = fresnel + (1.0f - fresnel) * (cosTerm * cosTerm * cosTerm * cosTerm * cosTerm);

		//ǥ�� ��ĥ�⸦ ǥ��(�ݿ� ������ ���)
		float3 halfway = normalize(normalize(cameraPosition - pin.posW) + L);
		float m = (1.0f - roughness) * 32.0f;
		roughnessTerm = (m + 8.0f) * pow(max(dot(halfway, pin.normal), 0.0f), m) / 8.0f;
		
		color += float4(rambertTerm * lights[i].color * (diffuse + fresnelTerm*roughnessTerm), 1.0f);
	}
	
	return color;
}
