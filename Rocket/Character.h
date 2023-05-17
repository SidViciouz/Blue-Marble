#pragma once

#include "Util.h"
#include "fbxsdk.h"

struct ControlPoint
{
	struct position
	{
		union
		{
			struct
			{
				float x;
				float y;
				float z;
			};

			float data[3];
		};
	} position;

	struct normal
	{
		union
		{
			struct
			{
				float x;
				float y;
				float z;
			};

			float data[3];
		};
	} normal;

	struct uv
	{
		union
		{
			struct
			{
				float x;
				float y;
			};

			float data[2];
		};
	} uv;
};

class SubMesh
{

public:
												SubMesh();

	void										SetMaterialIndex(int pIndex);

	void										AddIndex(int pIndex);

	void										AddTriangle(int pNum = 1);
	int											GetTriangleCount() const;

protected:
	int											mTriangleCount;

	vector<int>									mIndices;

	int											mMaterialIndex;
};

class SkeletalMesh
{
	int											mTriangle[6] = { 0,1,2,0,2,3 };

public:
												SkeletalMesh(FbxMesh* pMesh);

	void										Load(int pMaterialCount);

protected:

	FbxMesh*									mFbxMesh;
	vector<shared_ptr<SubMesh>>					mSubMeshes;
	vector<ControlPoint>						mControlPoints;
};

class Material
{
public:
												Material(FbxSurfaceMaterial* pMaterial);
	void										Print();

protected:
	void										Initialize(FbxSurfaceMaterial* pMaterial);

	FbxDouble3									GetMaterialProperty(const FbxSurfaceMaterial* pMaterial,
											const char* pPropertyName, const char* pFactorPropertyName);

	FbxDouble3									mEmissive;
	FbxDouble3									mAmbient;
	FbxDouble3									mDiffuse;
	FbxDouble3									mSpecular;
};

class Skeletal
{
public:
												Skeletal(const char* pSkeletalMeshPath);

	void										Load(FbxManager* pFbxManager);

protected:

	void										Print(FbxNode* pObj, int pTabs);
	void										PrintAttributeType(FbxNodeAttribute::EType pType);

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

	/*
	* draw 관련(임시)
	*/
	void										Upload();
	void										Draw();
	int											mVertexBuffer;
	int											mIndexBuffer;
	D3D12_VERTEX_BUFFER_VIEW					mVertexBufferView;
	D3D12_INDEX_BUFFER_VIEW						mIndexBufferView;
	D3D12_VERTEX_BUFFER_VIEW*					GetVertexBufferView();
	D3D12_INDEX_BUFFER_VIEW*					GetIndexBufferView();
	/*
	* draw 관련 end
	*/

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
