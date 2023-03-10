/*
* depth Map을 이용해서 particle을 생성해 particleMap에 업로드한다.
*/

Texture2DArray depthMap : register(t0);

RWTexture2DArray<float> particleMap : register(u0);


[numthreads(32,32,1)]
void CS()
{
	

}