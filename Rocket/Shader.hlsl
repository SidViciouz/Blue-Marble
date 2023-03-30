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

texture2D textureMap : register(t0);

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

VertexOut DistortionVS(VertexIn vin)
{
	VertexOut vout;

	float3 surface = normalize(vin.pos - cameraPosition);

	float cosOriginal = dot(cameraFront,surface);
	float distortedAngle = acos((cosOriginal - 0.96) / (1 - cosOriginal * 0.96));

	float3 axis = cross(surface, cameraFront);
	float dAngle = acos(cosOriginal) - distortedAngle;
	float c = cos(dAngle);
	float s = sin(dAngle);
	float rc = 1.0f - c;

	float4x4 rotation = {
		c + axis.x * axis.x*rc, axis.x*axis.y*rc -axis.z*s, axis.x*axis.z*rc + axis.y*s,0,
		axis.y*axis.x*rc + axis.z*s, c + axis.y*axis.y*rc, axis.y*axis.z*rc - axis.x*s,0,
		axis.z*axis.x*rc -axis.y*s, axis.z*axis.y*rc + axis.x*s, c + axis.z*axis.z*rc,0,
		0,0,0,1
	};

	float4 posW = mul(float4(vin.pos, 1.0f), transpose(world));
	
	float4 rotated = mul(posW,rotation);

	float4 posH = mul(mul(rotated, transpose(view)), transpose(projection));

	//homogeneous clip space�� ps�� ��������.(SV_POSITION semantic���� ������ ��)
	//vout.pos = mul(posW, viewProjection);
	vout.pos = posH;
	
	//�ݿ� ������ halfway vector�� ����ϱ� ���ؼ� �ʿ���.
	vout.posW = posW.xyz;

	//non uniform scaling�̶�� ���������� world matrix�� ���Ѵ�. �׷��� ���� ��쿡�� inverse-transpose�� �̿��ؾ���.
	//�� ��꿡 �̿��� ���̹Ƿ� world space�� ��ȯ
	vout.normal = mul(vin.normal,(float3x3)transpose(world));
	
	//uv��ǥ��
	vout.tex =vin.tex;

	return vout;
}

VertexOut SelectedVS(VertexIn vin)
{
	VertexOut vout;

	float4x4 scaledMatrix = {
		1.1f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.1f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.1f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};

	float4 scaled = mul(float4(vin.pos, 1.0f), scaledMatrix);

	float4 posW = mul(scaled, transpose(world));

	float4 posH = mul(mul(posW, transpose(view)), transpose(projection));

	vout.pos = posH;

	return vout;
}


float4 PS(VertexOut pin) : SV_Target
{
	float3 L = {0.0f,0.0f,1.0f};
	float rambertTerm = 0.0f;
	float4 color = float4(diffuseAlbedo * float3(0.1f, 0.1f, 0.1f), 1.0f);
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
		
		float3 diffuse = diffuseAlbedo * textureMap.Sample(textureSampler, pin.tex);

		color += float4(rambertTerm * lights[i].color * (diffuse + fresnelTerm*roughnessTerm), 1.0f);
	}

	return color;
}



float4 SelectedPS(VertexOut pin) : SV_Target
{
	return float4(0.9f,0.9f,0.3f,1.0f);
}