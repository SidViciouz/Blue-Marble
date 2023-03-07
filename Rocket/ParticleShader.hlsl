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

	for (int i = -1; i < 2; i++)
	{
		for (int j = -1; j < 2; j++)
		{
			for (int k = -1; k < 2; k++)
			{
				posX = (int)vin.position.x + i;
				posY = (int)vin.position.y + j;
				posZ = (int)vin.position.z + k;
				if (posX < 0 || posX > 10 || posY < 0 || posY > 10 || posZ < 0 || posZ > 10)
					continue;
				pos = int3(posX, posY, posZ);

				if (i == 0 && j == 0 && k == 0)
					InterlockedAdd(textureMap[pos], 5);
				//else if(i == 0)
				//	InterlockedAdd(textureMap[pos], 1);
				//else if((i == 0 && j == 0 && k != 0) && (i != 0 && j == 0 && k == 0) && (i == 0 && j != 0 && k == 0))
				else
					InterlockedAdd(textureMap[pos], 1);
			}
		}
	}
}