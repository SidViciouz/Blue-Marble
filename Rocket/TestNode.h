#pragma once

#include "SceneNode.h"
#include <fbxsdk.h>

class TestNode : public SceneNode
{
public:
												TestNode();
	/*
	* 이 노드를 먼저 그리고 자식 노드들을 그린다.
	*/
	virtual void								Draw() override;
	/*
	* 프레임마다 데이터를 업데이트한다.
	*/
	virtual void								Update() override;

	void										Print(FbxNode* pObj, int pTabs);

	D3D12_VERTEX_BUFFER_VIEW*					GetVertexBufferView();
	D3D12_INDEX_BUFFER_VIEW*					GetIndexBufferView();

	void										PlayStart(int index);
	void										PlayEnd();

protected:

	void										PrintAttributeType(FbxNodeAttribute::EType pType);
	void										PrintMesh(FbxMesh* pMesh,int pTabs);

	void										LoadMaterialData(FbxScene* pScene);
	FbxDouble3									GetMaterialProperty(const FbxSurfaceMaterial* pMaterial,
		const char* pPropertyName, const char* pFactorPropertyName, unsigned int& pTextureName);

	void										LoadVertexData(FbxScene* pScene);
	bool										SetCurrentAnimStack(FbxScene* pScene, int pIndex);
	void										DrawNodeRecursive(FbxNode* pNode, FbxTime& pTime, FbxAnimLayer* pAnimLayer,
		FbxAMatrix& pParentGlobalPositoin, FbxPose* pPose);
	FbxAMatrix									GetGlobalPosition(FbxNode* pNode, const FbxTime& pTime, FbxPose* pPose = nullptr,
		FbxAMatrix* pParentGlobalPosition = nullptr);
	FbxAMatrix									GetPoseMatrix(FbxPose* pPose, int pNodeIndex);
	void										TimerTick();
	FbxAMatrix									GetGeometry(FbxNode* pNode);
	void										DrawNode(
		FbxNode* pNode, FbxTime& pTime, FbxAnimLayer* pAnimLayer, FbxAMatrix& pParentGlobalPosition,
		FbxAMatrix& pGlobalPosition, FbxPose* pPose);
	void										DrawMesh(FbxNode* pNode, FbxTime& pTime, FbxAnimLayer* pAnimLayer,
		FbxAMatrix& pGlobalPosition, FbxPose* pPose);

	void										ReadVertexCacheData(FbxMesh* pMesh, FbxTime& pTime, FbxVector4* pVertexArray);
	void										UpdateVertexPosition(const FbxMesh* pMesh, const FbxVector4* pVertices);

	void										ComputeSkinDeformation(
		FbxAMatrix& pGlobalPosition,
		FbxMesh* pMesh,
		FbxTime& pTime,
		FbxVector4* pVertexArray,
		FbxPose* pPose);

	void										ComputeLinearDeformation(
		FbxAMatrix& pGlobalPosition,
		FbxMesh* pMesh,
		FbxTime& pTime,
		FbxVector4* pVertexArray,
		FbxPose* pPose);
	void										ComputeClusterDeformation(FbxAMatrix& pGlobalPosition,
		FbxMesh* pMesh, FbxCluster* pCluster, FbxAMatrix& pVertexTransformMatrix, FbxTime pTime, FbxPose* pPose);

	void										MatrixScale(FbxAMatrix& pMatrix, double pValue);

	void										MatrixAddToDiagonal(FbxAMatrix& pMatrix, double pValue);

	void										MatrixAdd(FbxAMatrix& pDstMatrix, FbxAMatrix& pSrcMatrix);

	void										ComputeDualQuaternionDeformation(FbxAMatrix& pGlobalPosition,
		FbxMesh* pMesh, FbxTime& pTime, FbxVector4* pVertexArray, FbxPose* pPose);

	const char* filename;
	FbxManager* sdkManager;
	FbxIOSettings* ios;
	FbxScene* scene1;
	FbxScene* mCurrentScene;
	FbxAnimEvaluator* mySceneEvaluator;

	/*
	* mesh data
	*/
	vector<Vertex>								mVertex;
	vector<uint16_t>							mIndex;
	int											mVertexBuffer;
	int											mIndexBuffer;
	D3D12_VERTEX_BUFFER_VIEW					mVertexBufferView;
	D3D12_INDEX_BUFFER_VIEW						mIndexBufferView;


	FbxArray<FbxString*>						mAnimStackNameArray;
	FbxAnimLayer*								mCurrentAnimLayer;
	FbxTime										mStart;
	FbxTime										mStop;
	FbxTime										mCurrentTime;
	FbxTime										mFrameTime;
	FbxTime										mCacheStart;
	FbxTime										mCacheStop;
	int											mPoseIndex = -1;

	bool										mAllByControlPoint = true;
	bool										mMeshCache = false;
	FbxVector4* lVertexArray = NULL;

	bool										mIsPlayed = false;
};