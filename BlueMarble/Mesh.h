#pragma once
#include "Util.h"
#include "Constant.h"

class Mesh
{
public:
												Mesh(string name, const char* fileName);
	/*
	* �����ϰ� �ִ� mesh�� �׸���.
	*/
	void										Draw();
	/*
	* mesh�� vertex buffer view�� ��ȯ�Ѵ�.
	*/
	D3D12_VERTEX_BUFFER_VIEW*					GetVertexBufferView();
	/*
	* mesh�� index buffer view�� ��ȯ�Ѵ�.
	*/
	D3D12_INDEX_BUFFER_VIEW*					GetIndexBufferView();
	/*
	* mesh�� ������ �� �ִ� �̸��̴�.
	*/
	string										mName;
	
	/*
	* mesh�� ���δ� bounding box�� ��Ÿ����.
	*/
	BoundingOrientedBox							mBound;

private:
	/*
	* mesh�� ������ wavefront(.obj)�������κ��� �ε��� �� ��ū�� �и��ϴµ� ���ȴ�.
	*/
	string										getToken(string& aLine, bool isFirst);
	/*
	* ��ū�� ���ڼ��� ��ȯ�Ѵ�.
	*/
	int											getNumber(string& aWord, bool isFirst);
	/*
	* mesh�����͸� ���Ϸκ��� �ε��ϰ� vertex, index �����͸� ä���.
	*/
	void										LoadFromFile(const char* fileName);
	/*
	* vertex�� index �������� �̿��Ͽ� tangent�� bitangent�� ���ϰ�, �̸� �����Ѵ�.
	*/
	void										CalculateTB();
	/*
	* vertex, index, tangent � ���� ������ ���ۿ� ���ε��Ѵ�.
	*/
	void										UploadBuffers();
	/*
	* mesh�� vertex buffer�� index(handle)�̴�.
	*/
	int											mVertexBufferIdx;
	/*
	* mesh�� index buffer�� index(handle)�̴�.
	*/
	int											mIndexBufferIdx;
	/*
	* mesh�� vertex upload buffer�� index(handle)�̴�.
	*/
	int											mVertexUploadBufferIdx;
	/*
	* mesh�� index upload buffer�� index(handle)�̴�.
	*/
	int											mIndexUploadBufferIdx;
	/*
	* mesh�� vertex �������� �����ϰ� �ִ�.
	*/
	vector<Vertex>								mVertices;
	/*
	* mesh�� index �������� �����ϰ� �ִ�.
	*/
	vector<uint16_t>							mIndices;
	/*
	* vertex buffer view�� �����ϰ� �ִ�.
	*/
	D3D12_VERTEX_BUFFER_VIEW					mVertexBufferView;
	/*
	* index buffer view�� �����ϰ� �ִ�.
	*/
	D3D12_INDEX_BUFFER_VIEW						mIndexBufferView;
	/*
	* tangent, bitangent buffer view�� �����ϰ� �ִ�.
	*/
	D3D12_VERTEX_BUFFER_VIEW					mTBBufferView;
	/*
	* vertex buffer view, tb buffer view�� �迭�� �����ϰ� �ִ�.
	*/
	D3D12_VERTEX_BUFFER_VIEW					mBufferViewArray[2];
	/*
	* vertex������ tangent�� bitangent�� �����ϴ� buffer�̴�.
	*/
	vector<TB>									mTBs;
	/*
	* tangent�� bitangent buffer�� index(handle)�̴�.
	*/
	int											mTBBufferIdx;
	/*
	* tangent�� bitangent upload buffer�� index(handle)�̴�.
	*/
	int											mTBUploadBufferIdx;
};
