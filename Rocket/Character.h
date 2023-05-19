#pragma once

#include "Util.h"
#include "fbxsdk.h"
#include "SceneNode.h"

enum class AnimationState
{
	eIdle,
	eWalking,
	eRunning
};

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

/*
*********************************************************************************
* SubMesh
*********************************************************************************
*/
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


/*
*********************************************************************************
* SkeletalMesh
*********************************************************************************
*/
class SkeletalMesh
{
	int											mTriangle[6] = { 0,1,2,0,2,3 };

public:
												SkeletalMesh(FbxMesh* pMesh);

	void										Load(int pMaterialCount);
	void										Draw();
	void										Upload();
	void										VertexUpload();

	int											GetControlPointCount() const;
	FbxMesh*									GetMesh() const;
	void										Deform(FbxAMatrix* pDeformation);

protected:

	FbxMesh*									mFbxMesh;
	vector<shared_ptr<SubMesh>>					mSubMeshes;
	vector<ControlPoint>						mControlPoints;

	D3D12_VERTEX_BUFFER_VIEW*					GetVertexBufferView();
	int											mVertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW					mVertexBufferView;
};

/*
*********************************************************************************
* Material
*********************************************************************************
*/

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

/*
*********************************************************************************
* Bone
*********************************************************************************
*/


class Bone
{
public:

using BoneIterator = std::vector<shared_ptr<Bone>>::const_iterator;
												Bone(const char* pName, FbxNode* pBoneNode);
	string										GetName() const;
	void										GetChildIterator(BoneIterator& pFirst, BoneIterator& pEnd) const;

protected:

	const char*									mName;

	FbxNode*									mNode;

	Bone*										mParentBone;
	vector<shared_ptr<Bone>>					mChildBones;
};


/*
*********************************************************************************
* Skeleton
*********************************************************************************
*/
class Skeleton
{
public:
												Skeleton(FbxSkeleton* pFbxSkeleton);

	void										ContrustBoneTree(FbxNode* pBoneNode);
	void										ConstructBoneMap(shared_ptr<Bone> pBone);

protected:
	FbxSkeleton*								mFbxSkeleton;

	shared_ptr<Bone>							mRootBone;

	unordered_map<string, shared_ptr<Bone>>		mBonesWithNames;
};

/*
*********************************************************************************
* Skeletal
*********************************************************************************
*/
class Skeletal
{
public:
												Skeletal(const char* pSkeletalMeshPath);

	void										Load(FbxManager* pFbxManager);
	void										Draw();
	void										Upload();
	void										VertexUpload();

	int											GetControlPointCount() const;
	FbxMesh*									GetMesh() const;
	int											GetClusterCount() const;
	void										Deform(FbxAMatrix* pDeformation);


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

	/*
	* skeleton 정보
	*/
	void										LoadSkeleton();
	shared_ptr<Skeleton>						mSkeleton;
};


/*
*********************************************************************************
* AnimationLayer
*********************************************************************************
*/
class AnimationLayer
{
public:
												AnimationLayer(const char* pPath,int pControlPointCount,FbxMesh* pFbxMesh,int pClusterCount);
	void										Load(FbxManager* pFbxManager);
	
	FbxAMatrix*									GetClusterDeformation(FbxTime pTime);
	FbxAMatrix*									GetControlPointDeformation(FbxTime pTime);

	FbxAMatrix									EvaluateBone(const char* pNodeName,FbxTime pTime);

	void										Tick(float pDeltaTime);

	FbxTime										GetTimeSpan() const;

	//시간이 오래걸리기 때문에 compute shader로 구현하는 것이 좋을 것 같다.
	//또는 getDeformation에서 cluster별로 matrix까지만 계산하고 controlPoint는 blend후에 넣는 방법도 가능할 것 같다.
	static FbxAMatrix*							ApplyAdditive(FbxAMatrix* pBase, FbxAMatrix* pAdditive,float pAlpha,int pSize);
	static FbxAMatrix*							Blend(AnimationLayer* pA, AnimationLayer* pB, float pAlpha, int pSize,double pTime);

protected: 

	shared_ptr<FbxAMatrix[]>					mControlPointDeformations;
	shared_ptr<FbxAMatrix[]>					mClusterDeformations;

	const char*									mAnimationLayerPath;

	FbxScene*									mFbxScene;

	FbxAnimLayer*								mAnimLayer;

	int											mControlPointCount;
	int											mClusterCount;
	FbxMesh*									mFbxMesh;

	FbxTime										mStart;
	FbxTime										mEnd;
	FbxTime										mCurrentTime;
};

/*
*********************************************************************************
* Character
*********************************************************************************
*/
class Character : public SceneNode
{
public:
												Character();

	void										AddAnimationLayer(const char* pAnimationPath,const char* pName);

	void										Initialize(const char* pSkeletalMeshPath);

	virtual void								Draw() override;
	void										Upload();
	void										VertexUpload();
	virtual void								Update() override;
	/*
	* Animation update
	*/
	void										UpdateAnimation();
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
	void										LoadSkeletal(const char* pSkeletalMeshPath);
	shared_ptr<Skeletal>						mSkeletal;

	/*
	* animation 정보
	*/
	vector<pair<const char*,const char*>>		mAnimationLayerPaths;
	unordered_map<string,shared_ptr<AnimationLayer>>	mAnimationLayers;
	void										LoadAnimationLayer();

	/*
	* Animation state
	*/
	AnimationState								mAnimationState;

	double										mStart;
	double										mEnd;
	double										mCurrentTime;
};
