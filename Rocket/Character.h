#pragma once

#include "Util.h"
#include "fbxsdk.h"
#include "SceneNode.h"

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
};

class SubMesh
{

public:
												SubMesh();

	void										SetMaterialIndex(int pIndex);

	void										AddIndex(uint16_t pIndex);

	void										AddTriangle(int pNum = 1);
	int											GetTriangleCount() const;
	void										Draw();
	void										Upload();

protected:
	int											mTriangleCount;

	vector<uint16_t>							mIndices;

	int											mMaterialIndex;

	int											mIndexBuffer;
	D3D12_INDEX_BUFFER_VIEW						mIndexBufferView;
	D3D12_INDEX_BUFFER_VIEW*					GetIndexBufferView();
};

class SkeletalMesh
{
	int											mTriangle[6] = { 0,1,2,0,2,3 };

public:
												SkeletalMesh(FbxMesh* pMesh);

	void										Load(int pMaterialCount);
	void										Draw();
	void										Upload();

protected:

	FbxMesh*									mFbxMesh;
	vector<shared_ptr<SubMesh>>					mSubMeshes;
	vector<ControlPoint>						mControlPoints;

	D3D12_VERTEX_BUFFER_VIEW*					GetVertexBufferView();
	int											mVertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW					mVertexBufferView;
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
	void										Draw();
	void										Upload();

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

class Character : public SceneNode
{
public:
												Character();

	void										AddAnimationLayer(const char* pAnimationPath);

	void										Initialize(const char* pSkeletalMeshPath);

	virtual void								Draw() override;
	void										Upload();

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
