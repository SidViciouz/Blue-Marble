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
	* skeletal mesh 정보
	*/
	void										LoadSkeletalMesh();
	const char*									mSkeletalMeshPath;
	shared_ptr<SkeletalMesh>					mSkeletalMesh;

	/*
	* material 정보
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
	* fbx sdk 관련 objects
	*/
	void										InitializeFbxSdkObject();
	FbxManager*									mFbxManager;
	FbxIOSettings*								mFbxIOSettings;

	/*
	* skeletal 정보 (mesh, material)
	*/
	shared_ptr<Skeletal>						mSkeletal;
	void										LoadSkeletal(const char* pSkeletalMeshPath);

	/*
	* animation 정보
	*/
	vector<const char*>							mAnimationLayerPaths;
	vector<shared_ptr<AnimationLayer>>			mAnimationLayers;
};
