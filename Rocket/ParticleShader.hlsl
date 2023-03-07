struct VertexIn
{
	float3 position : POSITION;
	float3 velocity : VELOCITY;
};

RWTexture3D<int> textureMap : register(u0);

void VS(VertexIn vin)
{
	int3 pos;
	int posX, posY, posZ;

	for (int i = -2; i < 3; i++)
	{
		for (int j = -2; j < 3; j++)
		{
			for (int k = -2; k < 3; k++)
			{
				posX = (int)vin.position.x + i;
				posY = (int)vin.position.y + j;
				posZ = (int)vin.position.z + k;
				if (posX < 0 || posX > 30 || posY < 0 || posY > 30 || posZ < 0 || posZ > 30)
					continue;
				pos = int3(posX, posY, posZ);

				if (i == 0 && j == 0 && k == 0)
					InterlockedAdd(textureMap[pos], 7);
				else if ((i == 0 && j == 0 && k != 0) || (i != 0 && j == 0 && k == 0) || (i == 0 && j != 0 && k == 0))
					InterlockedAdd(textureMap[pos], 5);
				else if ((i > -2 && i < 2) || (j > -2 && j < 2) || (k > -2 && k < 2))
					InterlockedAdd(textureMap[pos], 3);
			}
		}
	}
}