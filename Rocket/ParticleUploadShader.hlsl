/*
* depth Map�� �̿��ؼ� particle�� ������ particleMap�� ���ε��Ѵ�.
*/

Texture2DArray depthMap : register(t0);

RWTexture2DArray<float> particleMap : register(u0);


[numthreads(32,32,1)]
void CS()
{
	

}