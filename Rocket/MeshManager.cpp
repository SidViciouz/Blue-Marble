#include "MeshManager.h"
#include "IfError.h"
#include <fstream>

void MeshManager::Load(string name, const char* fileName, bool hasRigidBody)
{
	mMeshes[name] = make_unique<Mesh>(name,fileName,hasRigidBody);
}

void MeshManager::Draw(string name)
{
	mMeshes[name]->Draw();
}