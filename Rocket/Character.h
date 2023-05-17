#pragma once

#include "Util.h"
#include "fbxsdk.h"

class SkeletalMesh
{
public:
												SkeletalMesh();

	void										Load();
};

class Material
{

};

class Skeletal
{
public:
												Skeletal(const char* pSkeletalMeshPath);

	void										Load(FbxManager* pFbxManager);

protected:

	void										LoadScene(FbxManager* pFbxManager);

	FbxScene*									mFbxScene;
	/*
	* skeletal mesh ����
	*/
	void										LoadSkeletalMesh();
	const char*									mSkeletalMeshPath;
	shared_ptr<SkeletalMesh>					mSkeletalMesh;

	/*
	* material ����
	*/
	void										LoadMaterials();
	vector<shared_ptr<Material>>				mMaterials;
};



class AnimationLayer
{

};

class Character
{
public:
												Character();

	void										AddAnimationLayer(const char* pAnimationPath);

	void										Initialize(const char* pSkeletalMeshPath);

protected:

	/*
	* fbx sdk ���� objects
	*/
	void										InitializeFbxSdkObject();
	FbxManager*									mFbxManager;
	FbxIOSettings*								mFbxIOSettings;

	/*
	* skeletal ���� (mesh, material)
	*/
	shared_ptr<Skeletal>						mSkeletal;
	void										LoadSkeletal(const char* pSkeletalMeshPath);

	/*
	* animation ����
	*/
	vector<const char*>							mAnimationLayerPaths;
	vector<shared_ptr<AnimationLayer>>			mAnimationLayers;
};
