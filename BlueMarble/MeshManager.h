#pragma once

#include "Util.h"
#include "Mesh.h"

using Meshes = unordered_map<string, unique_ptr<Mesh>>;

class MeshManager
{
public:
	/*
	* mesh를 정해진 로드해서 저장한다.
	*/
	void										Load(string name,const char* fileName);
	/*
	* 입력된 이름의 mesh를 그린다.
	*/
	void										Draw(string name);
	/*
	* mesh들의 정보를 저장하고 있는 자료구조이다.
	*/
	Meshes										mMeshes;
};
