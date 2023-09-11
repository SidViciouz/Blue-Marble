#pragma once

#include "Util.h"
#include "Mesh.h"

using Meshes = unordered_map<string, unique_ptr<Mesh>>;

class MeshManager
{
public:
	/*
	* mesh�� ������ �ε��ؼ� �����Ѵ�.
	*/
	void										Load(string name,const char* fileName);
	/*
	* �Էµ� �̸��� mesh�� �׸���.
	*/
	void										Draw(string name);
	/*
	* mesh���� ������ �����ϰ� �ִ� �ڷᱸ���̴�.
	*/
	Meshes										mMeshes;
};
