#pragma once
#include "Util.h"
#include "Constant.h"

class Mesh
{
public:
												Mesh(string name, const char* fileName, bool hasRigidBody);
	void										Draw();
	D3D12_VERTEX_BUFFER_VIEW*					GetVertexBufferView();
	D3D12_INDEX_BUFFER_VIEW*					GetIndexBufferView();

	string										mName;
	int											mVertexBufferIdx;
	int											mIndexBufferIdx;
	int											mVertexUploadBufferIdx;
	int											mIndexUploadBufferIdx;
	vector<Vertex>								mVertices;
	vector<uint16_t>							mIndices;
	D3D12_VERTEX_BUFFER_VIEW					mVertexBufferView;
	D3D12_INDEX_BUFFER_VIEW						mIndexBufferView;
	BoundingOrientedBox							mBound;
	bool										mHasRigidBody;

private:
	string										getToken(string& aLine, bool isFirst);
	int											getNumber(string& aWord, bool isFirst);
};
