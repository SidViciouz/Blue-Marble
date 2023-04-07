#pragma once

#include "Util.h"
#include "Mesh.h"

using Meshes = unordered_map<string, unique_ptr<Mesh>>;

class MeshManager
{
public:
	void										Load(string name,const char* fileName);
	void										Draw(string name);

	Meshes										mMeshes;
};
