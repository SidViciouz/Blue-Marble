#pragma once
#include "Util.h"
#include "Constant.h"

class Mesh
{
public:
												Mesh(string name, const char* fileName);
	/*
	* 저장하고 있는 mesh를 그린다.
	*/
	void										Draw();
	/*
	* mesh의 vertex buffer view를 반환한다.
	*/
	D3D12_VERTEX_BUFFER_VIEW*					GetVertexBufferView();
	/*
	* mesh의 index buffer view를 반환한다.
	*/
	D3D12_INDEX_BUFFER_VIEW*					GetIndexBufferView();
	/*
	* mesh를 접근할 수 있는 이름이다.
	*/
	string										mName;
	
	/*
	* mesh의 감싸는 bounding box를 나타낸다.
	*/
	BoundingOrientedBox							mBound;

private:
	/*
	* mesh의 정보를 wavefront(.obj)포맷으로부터 로드할 떄 토큰을 분리하는데 사용된다.
	*/
	string										getToken(string& aLine, bool isFirst);
	/*
	* 토큰의 글자수를 반환한다.
	*/
	int											getNumber(string& aWord, bool isFirst);
	/*
	* mesh데이터를 파일로부터 로드하고 vertex, index 데이터를 채운다.
	*/
	void										LoadFromFile(const char* fileName);
	/*
	* vertex와 index 정보들을 이용하여 tangent와 bitangent를 구하고, 이를 저장한다.
	*/
	void										CalculateTB();
	/*
	* vertex, index, tangent 등에 대한 정보를 버퍼에 업로드한다.
	*/
	void										UploadBuffers();
	/*
	* mesh의 vertex buffer의 index(handle)이다.
	*/
	int											mVertexBufferIdx;
	/*
	* mesh의 index buffer의 index(handle)이다.
	*/
	int											mIndexBufferIdx;
	/*
	* mesh의 vertex upload buffer의 index(handle)이다.
	*/
	int											mVertexUploadBufferIdx;
	/*
	* mesh의 index upload buffer의 index(handle)이다.
	*/
	int											mIndexUploadBufferIdx;
	/*
	* mesh의 vertex 정보들을 저장하고 있다.
	*/
	vector<Vertex>								mVertices;
	/*
	* mesh의 index 정보들을 저장하고 있다.
	*/
	vector<uint16_t>							mIndices;
	/*
	* vertex buffer view를 저장하고 있다.
	*/
	D3D12_VERTEX_BUFFER_VIEW					mVertexBufferView;
	/*
	* index buffer view를 저장하고 있다.
	*/
	D3D12_INDEX_BUFFER_VIEW						mIndexBufferView;
	/*
	* tangent, bitangent buffer view를 저장하고 있다.
	*/
	D3D12_VERTEX_BUFFER_VIEW					mTBBufferView;
	/*
	* vertex buffer view, tb buffer view의 배열을 저장하고 있다.
	*/
	D3D12_VERTEX_BUFFER_VIEW					mBufferViewArray[2];
	/*
	* vertex마다의 tangent와 bitangent를 저장하는 buffer이다.
	*/
	vector<TB>									mTBs;
	/*
	* tangent와 bitangent buffer의 index(handle)이다.
	*/
	int											mTBBufferIdx;
	/*
	* tangent와 bitangent upload buffer의 index(handle)이다.
	*/
	int											mTBUploadBufferIdx;
};
