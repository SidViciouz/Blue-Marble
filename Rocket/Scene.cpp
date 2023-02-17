#include "Scene.h"

void Scene::CreateVertexIndexBuffer(Pipeline& pipeline)
{
	mVertexBuffer = make_unique<Buffer>(pipeline.GetDevice(), sizeof(Vertex) * mAllVertices.size());
	mIndexBuffer = make_unique<Buffer>(pipeline.GetDevice(), sizeof(uint16_t) * mAllIndices.size());

	//Vertex, Index ���ۿ� Model ������ copy
	mVertexBuffer->Copy(mAllVertices.data(), sizeof(Vertex) * mAllVertices.size(), pipeline.GetCommandList());
	mIndexBuffer->Copy(mAllIndices.data(), sizeof(uint16_t) * mAllIndices.size(), pipeline.GetCommandList());
}
