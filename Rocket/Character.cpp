#include "Character.h"
#include "Engine.h"

/*
*********************************************************************************
* Material
*********************************************************************************
*/

Material::Material(FbxSurfaceMaterial* pMaterial)
{
    Initialize(pMaterial);
}

void Material::Print()
{
    printf("emissive : %f %f %f\n", mEmissive[0], mEmissive[1], mEmissive[2]);
    printf("mAmbient : %f %f %f\n", mAmbient[0], mAmbient[1], mAmbient[2]);
    printf("mDiffuse : %f %f %f\n", mDiffuse[0], mDiffuse[1], mDiffuse[2]);
    printf("mSpecular : %f %f %f\n\n", mSpecular[0], mSpecular[1], mSpecular[2]);
}

void Material::Initialize(FbxSurfaceMaterial* pMaterial)
{
    mEmissive = GetMaterialProperty(pMaterial,
        FbxSurfaceMaterial::sEmissive, FbxSurfaceMaterial::sEmissiveFactor);

    mAmbient = GetMaterialProperty(pMaterial,
        FbxSurfaceMaterial::sAmbient, FbxSurfaceMaterial::sAmbientFactor);

    mDiffuse = GetMaterialProperty(pMaterial,
        FbxSurfaceMaterial::sDiffuse, FbxSurfaceMaterial::sDiffuseFactor);

    mSpecular = GetMaterialProperty(pMaterial,
        FbxSurfaceMaterial::sSpecular, FbxSurfaceMaterial::sSpecularFactor);
}

FbxDouble3 Material::GetMaterialProperty(const FbxSurfaceMaterial* pMaterial,
    const char* pPropertyName, const char* pFactorPropertyName)
{
    FbxDouble3 lResult(0, 0, 0);
    const FbxProperty lProperty = pMaterial->FindProperty(pPropertyName);
    const FbxProperty lFactorProeprty = pMaterial->FindProperty(pFactorPropertyName);
    if (lProperty.IsValid() && lFactorProeprty.IsValid())
    {
        lResult = lProperty.Get<FbxDouble3>();
        double lFactor = lFactorProeprty.Get<FbxDouble>();
        if (lFactor != 1)
        {
            lResult[0] *= lFactor;
            lResult[1] *= lFactor;
            lResult[2] *= lFactor;
        }
    }

    return lResult;
}


/*
*********************************************************************************
* SubMesh
*********************************************************************************
*/

SubMesh::SubMesh()
    : mTriangleCount{ 0 }, mMaterialIndex{ -1 }
{

}

void SubMesh::SetMaterialIndex(int pIndex)
{
    mMaterialIndex = pIndex;
}

void SubMesh::AddIndex(uint16_t pIndex)
{
    mIndices.push_back(pIndex);
}

void SubMesh::AddTriangle(int pNum)
{
    mTriangleCount += pNum;
}

int	SubMesh::GetTriangleCount() const
{
    return mTriangleCount;
}

void SubMesh::Draw()
{
    Engine::mCommandList->IASetIndexBuffer(GetIndexBufferView());
    Engine::mCommandList->DrawIndexedInstanced(mIndices.size(), 1, 0, 0, 0);
}

void SubMesh::Upload()
{
    mIndexBuffer = Engine::mResourceManager->CreateUploadBuffer(sizeof(uint16_t) * mIndices.size());
    Engine::mResourceManager->Upload(mIndexBuffer, mIndices.data(), sizeof(uint16_t) * mIndices.size(), 0);
}

D3D12_INDEX_BUFFER_VIEW* SubMesh::GetIndexBufferView()
{
    mIndexBufferView.BufferLocation = Engine::mResourceManager->GetResource(mIndexBuffer)->GetGPUVirtualAddress();
    mIndexBufferView.Format = DXGI_FORMAT_R16_UINT;
    mIndexBufferView.SizeInBytes = sizeof(uint16_t) * mIndices.size();

    return &mIndexBufferView;
}

/*
*********************************************************************************
* SkeletalMesh
*********************************************************************************
*/

SkeletalMesh::SkeletalMesh(FbxMesh* pMesh)
    : mFbxMesh{ pMesh }
{

}

void SkeletalMesh::Load(int pMaterialCount)
{
    /*
    * check mapping mode of uv, normal, material
    */
    FbxLayerElement::EMappingMode lUvMappingMode = mFbxMesh->GetElementUV(0)->GetMappingMode();
    FbxLayerElement::EMappingMode lNormalMappingMode = mFbxMesh->GetElementNormal(0)->GetMappingMode();
    FbxLayerElement::EMappingMode lMaterialMappingMode = mFbxMesh->GetElementMaterial(0)->GetMappingMode();

    printf("uv mapping mode is : %d\n", lUvMappingMode);
    printf("normal mapping mode is : %d\n", lNormalMappingMode);
    printf("material mapping mode is : %d\n", lMaterialMappingMode);

    //mapping mode other than this is not supported yet
    if (lUvMappingMode != FbxLayerElement::EMappingMode::eByControlPoint ||
        lNormalMappingMode != FbxLayerElement::EMappingMode::eByControlPoint ||
        lMaterialMappingMode != FbxLayerElement::EMappingMode::eByPolygon)
    {
        printf("not supported mapping mode!\n");
        return;
    }

    const FbxGeometryElementNormal* lElementNormal = mFbxMesh->GetElementNormal(0);
    const FbxGeometryElementUV* lElementUv = mFbxMesh->GetElementUV(0);

    /*
    * store control point, normal, uv
    */
    FbxVector4* lControlPoints = mFbxMesh->GetControlPoints();

    int lControlPointCount = mFbxMesh->GetControlPointsCount();
    for (int lControlPointIndex = 0; lControlPointIndex < lControlPointCount; ++lControlPointIndex)
    {
        ControlPoint lControlPoint;

        int lNormalIndex = lControlPointIndex;
        if (lElementNormal->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
            lNormalIndex = lElementNormal->GetIndexArray().GetAt(lControlPointIndex);
        FbxVector4 lNormal = lElementNormal->GetDirectArray().GetAt(lNormalIndex);

        int lUVIndex = lControlPointIndex;
        if (lElementUv->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
            lUVIndex = lElementUv->GetIndexArray().GetAt(lControlPointIndex);
        FbxVector2 lUv = lElementUv->GetDirectArray().GetAt(lUVIndex);

        for (int lIndex = 0; lIndex < 3; ++lIndex)
        {
            lControlPoint.position.data[lIndex] = lControlPoints[lControlPointIndex][lIndex];

            lControlPoint.normal.data[lIndex] = lNormal[lIndex];

            if (lIndex != 2)
                lControlPoint.uv.data[lIndex] = lUv[lIndex];
        }

        mControlPoints.push_back(lControlPoint);
    }

    /*
    * store subMesh data
    */
    for (int lMaterialIndex = 0; lMaterialIndex < pMaterialCount; ++lMaterialIndex)
    {
        mSubMeshes.push_back(make_shared<SubMesh>());
        mSubMeshes.back()->SetMaterialIndex(lMaterialIndex);
    }

    /*
    * store index data for every polygon
    */
    FbxLayerElementArrayTemplate<int>& lMaterialIndexArray = mFbxMesh->GetElementMaterial()->GetIndexArray();
    int lPolygonCount = mFbxMesh->GetPolygonCount();
    for (int lPolygonIndex = 0; lPolygonIndex < lPolygonCount; ++lPolygonIndex)
    {
        int lMaterialIndex = lMaterialIndexArray.GetAt(lPolygonIndex);

        int lPolygonSize = mFbxMesh->GetPolygonSize(lPolygonIndex);

        if (lPolygonSize == 3)
        {
            for (int lPolygonPosition = 0; lPolygonPosition < 3; ++lPolygonPosition)
            {
                int lIndex = mFbxMesh->GetPolygonVertex(lPolygonIndex, lPolygonPosition);
                mSubMeshes[lMaterialIndex]->AddIndex(static_cast<uint16_t>(lIndex));

            }

            mSubMeshes[lMaterialIndex]->AddTriangle();
        }
        else if (lPolygonSize == 4)
        {
            for (int i = 0; i < 6; ++i)
            {
                int lIndex = mFbxMesh->GetPolygonVertex(lPolygonIndex, mTriangle[i]);
                mSubMeshes[lMaterialIndex]->AddIndex(static_cast<uint16_t>(lIndex));

            }

            mSubMeshes[lMaterialIndex]->AddTriangle(2);
        }
        else
        {
            printf("lPolygonSize is %d\n", lPolygonSize);
        }
    }
}

void SkeletalMesh::Draw()
{
    Engine::mCommandList->IASetVertexBuffers(0, 1, GetVertexBufferView());

    for (auto lSubMesh : mSubMeshes)
    {
        lSubMesh->Draw();
    }
}

void SkeletalMesh::Upload()
{
    mVertexBuffer = Engine::mResourceManager->CreateUploadBuffer(sizeof(ControlPoint) * mControlPoints.size());

    VertexUpload();

    for (auto lSubMesh : mSubMeshes)
    {
        lSubMesh->Upload();
    }
}

void SkeletalMesh::VertexUpload()
{
    Engine::mResourceManager->Upload(mVertexBuffer, mControlPoints.data(), sizeof(ControlPoint) * mControlPoints.size(), 0);
}

int	SkeletalMesh::GetControlPointCount() const
{
    return mControlPoints.size();
}

D3D12_VERTEX_BUFFER_VIEW* SkeletalMesh::GetVertexBufferView()
{
    mVertexBufferView.BufferLocation = Engine::mResourceManager->GetResource(mVertexBuffer)->GetGPUVirtualAddress();
    mVertexBufferView.StrideInBytes = sizeof(ControlPoint);
    mVertexBufferView.SizeInBytes = sizeof(ControlPoint) * mControlPoints.size();

    return &mVertexBufferView;
}

FbxMesh* SkeletalMesh::GetMesh() const
{
    return mFbxMesh;
}

void SkeletalMesh::Deform(FbxAMatrix* pDeformation)
{
    int lControlPointCount = mControlPoints.size();
    for (int lControlPointIndex = 0; lControlPointIndex < lControlPointCount; ++lControlPointIndex)
    {
        FbxVector4 lPos = pDeformation[lControlPointIndex].MultT(mFbxMesh->GetControlPointAt(lControlPointIndex));
        
        mControlPoints[lControlPointIndex].position = { (float)lPos[0],(float)lPos[1],(float)lPos[2] };
    }
}

/*
*********************************************************************************
* Bone
*********************************************************************************
*/

Bone::Bone(const char* pName, FbxNode* pBoneNode)
    : mName{pName}, mNode{pBoneNode}
{
    printf("[%s]\n", pBoneNode->GetName());
    
    int lChildCount = pBoneNode->GetChildCount();
    for (int lChildIndex = 0; lChildIndex < lChildCount; ++lChildIndex)
    {
        FbxNode* lChild = pBoneNode->GetChild(lChildIndex);
        mChildBones.push_back(make_shared<Bone>(lChild->GetName(), lChild));
    }
}

string Bone::GetName() const
{
    return string(mName);
}

void Bone::GetChildIterator(BoneIterator& pFirst, BoneIterator& pEnd) const
{
    pFirst = mChildBones.cbegin();
    pEnd = mChildBones.cend();
}

/*
*********************************************************************************
* Skeleton
*********************************************************************************
*/

Skeleton::Skeleton(FbxSkeleton* pFbxSkeleton)
    : mFbxSkeleton{pFbxSkeleton}
{
    ContrustBoneTree(mFbxSkeleton->GetNode());
    ConstructBoneMap(mRootBone);
}

void Skeleton::ContrustBoneTree(FbxNode* pBoneNode)
{
    mRootBone = make_shared<Bone>(pBoneNode->GetName(),pBoneNode);
}

void Skeleton::ConstructBoneMap(shared_ptr<Bone> pBone)
{
    mBonesWithNames[pBone->GetName()] = pBone;

    Bone::BoneIterator lFirst, lEnd;

    pBone->GetChildIterator(lFirst, lEnd);

    for (auto lIter = lFirst; lIter != lEnd; ++lIter)
    {
        ConstructBoneMap(*lIter);
    }
}

/*
*********************************************************************************
* Skeletal
*********************************************************************************
*/

Skeletal::Skeletal(const char* pSkeletalMeshPath)
    : mSkeletalMeshPath{pSkeletalMeshPath}
{
    mFbxScene = nullptr;
}

void Skeletal::Load(FbxManager* pFbxManager)
{
    LoadScene(pFbxManager);

    LoadMaterials();

    LoadSkeletalMesh();

    LoadSkeleton();
}


void Skeletal::LoadScene(FbxManager* pFbxManager)
{
    FbxImporter* lImporter = FbxImporter::Create(pFbxManager, "");
    if (!lImporter->Initialize(mSkeletalMeshPath, -1, pFbxManager->GetIOSettings()))
    {
        printf("Call to FbxImporter::Initialize() failed.\n");
        printf("Error returned: %s\n\n", lImporter->GetStatus().GetErrorString());
        exit(-1);
    }
    mFbxScene = FbxScene::Create(pFbxManager, "myScene");
    lImporter->Import(mFbxScene);
    lImporter->Destroy();
}

void Skeletal::LoadMaterials()
{
    int lMaterialCount = mFbxScene->GetMaterialCount();
    for (int lMaterialIndex = 0; lMaterialIndex < lMaterialCount; ++lMaterialIndex)
    {
        FbxSurfaceMaterial* lMaterial = mFbxScene->GetMaterial(lMaterialIndex);
        mMaterials.push_back(make_shared<Material>(lMaterial));
    }
}

void Skeletal::LoadSkeletalMesh()
{
    FbxMesh* lMesh = 
        FbxCast<FbxMesh>(mFbxScene->GetSrcObject(FbxCriteria::ObjectType(FbxMesh::ClassId)));
    mSkeletalMesh = make_shared<SkeletalMesh>(lMesh);
    mSkeletalMesh->Load(mMaterials.size());
}

void Skeletal::LoadSkeleton()
{
    FbxSkeleton* lSkeleton = mFbxScene->GetSrcObject<FbxSkeleton>();
    mSkeleton = make_shared<Skeleton>(lSkeleton);
}

void Skeletal::Draw()
{
    mSkeletalMesh->Draw();
}

void Skeletal::Upload()
{
    mSkeletalMesh->Upload();
}

void Skeletal::VertexUpload()
{
    mSkeletalMesh->VertexUpload();
}

int	Skeletal::GetControlPointCount() const
{
    return mSkeletalMesh->GetControlPointCount();
}

FbxMesh* Skeletal::GetMesh() const
{
    return mSkeletalMesh->GetMesh();
}

int	Skeletal::GetClusterCount() const
{
    return FbxCast<FbxSkin>(GetMesh()->GetDeformer(0, FbxDeformer::EDeformerType::eSkin))->GetClusterCount();
}

void Skeletal::Deform(FbxAMatrix* pDeformation)
{
    mSkeletalMesh->Deform(pDeformation);
}

void Skeletal::Print(FbxNode* pObj, int pTabs)
{
    if (pObj->GetNodeAttributeCount())
    {
        FbxNodeAttribute* lAtt = pObj->GetNodeAttribute();

        FbxNodeAttribute::EType lType = lAtt->GetAttributeType();

        for (int i = 0; i < pTabs; ++i)
            printf("\t");
        printf("%s\t", lAtt->GetName());
        PrintAttributeType(lType);
    }

    int numChild = pObj->GetChildCount();
    for (int i = 0; i < numChild; ++i)
    {
        FbxNode* child = pObj->GetChild(i);
        Print(child, pTabs + 1);
    }

}

void Skeletal::PrintAttributeType(FbxNodeAttribute::EType pType)
{
    switch (pType)
    {
    case FbxNodeAttribute::EType::eUnknown:
        printf("Etype : eUnknown\n");
        break;

    case FbxNodeAttribute::EType::eNull:
        printf("Etype : eNull\n");
        break;

    case FbxNodeAttribute::EType::eMarker:
        printf("Etype : eMarker\n");
        break;

    case FbxNodeAttribute::EType::eSkeleton:
        printf("Etype : eSkeleton\n");
        break;

    case FbxNodeAttribute::EType::eMesh:
        printf("Etype : eMesh\n");
        break;

    case FbxNodeAttribute::EType::eNurbs:
        printf("Etype : eNurbs\n");
        break;

    case FbxNodeAttribute::EType::ePatch:
        printf("Etype : ePatch\n");
        break;

    case FbxNodeAttribute::EType::eCamera:
        printf("Etype : eCamera\n");
        break;

    case FbxNodeAttribute::EType::eCameraStereo:
        printf("Etype : eCameraStereo\n");
        break;

    case FbxNodeAttribute::EType::eCameraSwitcher:
        printf("Etype : eCameraSwitcher\n");
        break;

    case FbxNodeAttribute::EType::eLight:
        printf("Etype : eLight\n");
        break;

    case FbxNodeAttribute::EType::eOpticalReference:
        printf("Etype : eOpticalReference\n");
        break;

    case FbxNodeAttribute::EType::eOpticalMarker:
        printf("Etype : eOpticalMarker\n");
        break;

    case FbxNodeAttribute::EType::eNurbsCurve:
        printf("Etype : eNurbsCurve\n");
        break;

    case FbxNodeAttribute::EType::eTrimNurbsSurface:
        printf("Etype : eTrimNurbsSurface\n");
        break;

    case FbxNodeAttribute::EType::eBoundary:
        printf("Etype : eBoundary\n");
        break;

    case FbxNodeAttribute::EType::eNurbsSurface:
        printf("Etype : eNurbsSurface\n");
        break;

    case FbxNodeAttribute::EType::eShape:
        printf("Etype : eShape\n");
        break;

    case FbxNodeAttribute::EType::eLODGroup:
        printf("Etype : eLODGroup\n");
        break;

    case FbxNodeAttribute::EType::eSubDiv:
        printf("Etype : eSubDiv\n");
        break;

    case FbxNodeAttribute::EType::eCachedEffect:
        printf("Etype : eCachedEffect\n");
        break;

    case FbxNodeAttribute::EType::eLine:
        printf("Etype : eLine\n");
        break;
    }
}

/*
*********************************************************************************
* AnimationLayer
*********************************************************************************
*/

AnimationLayer::AnimationLayer(const char* pPath, int pControlPointCount,FbxMesh* pFbxMesh, int pClusterCount)
    : mAnimationLayerPath{pPath}, mControlPointCount{pControlPointCount}, mClusterCount{pClusterCount},
        mControlPointDeformations(new FbxAMatrix[pControlPointCount], std::default_delete<FbxAMatrix[]>()),
        mFbxMesh{pFbxMesh},
        mClusterDeformations(new FbxAMatrix[pClusterCount], std::default_delete<FbxAMatrix[]>())
{
}

void AnimationLayer::Load(FbxManager* pFbxManager)
{
    FbxImporter* lImporter = FbxImporter::Create(pFbxManager, "");
    if (!lImporter->Initialize(mAnimationLayerPath, -1, pFbxManager->GetIOSettings()))
    {
        printf("Call to FbxImporter::Initialize() failed.\n");
        printf("Error returned: %s\n\n", lImporter->GetStatus().GetErrorString());
        exit(-1);
    }
    mFbxScene = FbxScene::Create(pFbxManager, "myScene");
    lImporter->Import(mFbxScene);
    lImporter->Destroy();

    FbxArray<FbxString*> lAnimStackNameArray;
    mFbxScene->FillAnimStackNameArray(lAnimStackNameArray);
    int lAnimStackCount = lAnimStackNameArray.GetCount();
    if (!lAnimStackCount)
        return;

    FbxAnimStack* lAnimStack = mFbxScene->FindMember<FbxAnimStack>(lAnimStackNameArray[0]->Buffer());
    mFbxScene->SetCurrentAnimationStack(lAnimStack);

    mAnimLayer = lAnimStack->GetMember<FbxAnimLayer>();

    FbxTakeInfo* lTakeInfo = mFbxScene->GetTakeInfo(*(lAnimStackNameArray[0]));

    if (lTakeInfo == nullptr)
    {
        printf("no take info!\n");
        return;
    }

    mStart = lTakeInfo->mLocalTimeSpan.GetStart();
    mEnd = lTakeInfo->mLocalTimeSpan.GetStop();

    if (FBXSDK_TIME_INFINITE < mStart)
        mStart = FBXSDK_TIME_INFINITE;

    if (FBXSDK_TIME_MINUS_INFINITE > mEnd)
        mEnd = FBXSDK_TIME_MINUS_INFINITE;

    mCurrentTime = mStart;

    printf("current : %f\nstart : %f\nend: %f\n\n",
        mCurrentTime.GetSecondDouble(), mStart.GetSecondDouble(), mEnd.GetSecondDouble());
}

FbxAMatrix* AnimationLayer::GetClusterDeformation(FbxTime pTime)
{
    FbxAMatrix* lDeformation = mClusterDeformations.get();

    //calculate deformation matrices
    FbxSkeleton* lSkeleton = mFbxScene->GetSrcObject<FbxSkeleton>();
    if (lSkeleton == nullptr)
    {
        printf("lSkeleton is nullptr\n");
        return lDeformation;
    }

    FbxSkin* lSkin = FbxCast<FbxSkin>(mFbxMesh->GetDeformer(0, FbxDeformer::EDeformerType::eSkin));
    if (lSkin == nullptr)
    {
        printf("lSkin is nullptr\n");
        return lDeformation;
    }

    //printf("%d %d,\n", lSkin->GetSkinningType(), lSkin->GetCluster(0)->GetLinkMode());
    //rigid, and totalOne

    int lClusterCount = lSkin->GetClusterCount();
    for (int lClusterIndex = 0; lClusterIndex < lClusterCount; ++lClusterIndex)
    {
        FbxAMatrix lInitM;

        FbxCluster* lCluster = lSkin->GetCluster(lClusterIndex);

        FbxAMatrix lCurM = EvaluateBone(lCluster->GetLink()->GetName(),pTime);
        lCluster->GetTransformLinkMatrix(lInitM);
        FbxAMatrix lRelativeM = lCurM * lInitM.Inverse();

        int lCount = lCluster->GetControlPointIndicesCount();
        int* lIndices = lCluster->GetControlPointIndices();
        double* lWeights = lCluster->GetControlPointWeights();

        lDeformation[lClusterIndex] = lRelativeM;
    }

    return lDeformation;
}

FbxAMatrix* AnimationLayer::GetControlPointDeformation(FbxTime pTime)
{
    FbxAMatrix* lDeformation = mControlPointDeformations.get();

    //calculate deformation matrices
    FbxSkeleton* lSkeleton = mFbxScene->GetSrcObject<FbxSkeleton>();
    if (lSkeleton == nullptr)
    {
        printf("lSkeleton is nullptr\n");
        return lDeformation;
    }

    FbxSkin* lSkin = FbxCast<FbxSkin>(mFbxMesh->GetDeformer(0, FbxDeformer::EDeformerType::eSkin));
    if (lSkin == nullptr)
    {
        printf("lSkin is nullptr\n");
        return lDeformation;
    }

    //printf("%d %d,\n", lSkin->GetSkinningType(), lSkin->GetCluster(0)->GetLinkMode());
    //rigid, and totalOne

    int lClusterCount = lSkin->GetClusterCount();
    for (int lClusterIndex = 0; lClusterIndex < lClusterCount; ++lClusterIndex)
    {
        FbxAMatrix lInitM;

        FbxCluster* lCluster = lSkin->GetCluster(lClusterIndex);

        FbxAMatrix lCurM = EvaluateBone(lCluster->GetLink()->GetName(), pTime);
        lCluster->GetTransformLinkMatrix(lInitM);
        FbxAMatrix lRelativeM = lCurM * lInitM.Inverse();
        
        int lCount = lCluster->GetControlPointIndicesCount();
        int* lIndices = lCluster->GetControlPointIndices();
        double* lWeights = lCluster->GetControlPointWeights();
        
        for (int i = 0; i < lCount; ++i)
        {
            lDeformation[lIndices[i]] = lRelativeM * lWeights[i];
        }
        
    }
    
    return lDeformation;
}

FbxAMatrix* AnimationLayer::ApplyAdditive(FbxAMatrix* pBase, FbxAMatrix* pAdditive, float pAlpha, int pSize)
{
    return nullptr;
}

FbxAMatrix* AnimationLayer::Blend(AnimationLayer* pA, AnimationLayer* pB, float pAlpha, int pSize, double pTime)
{
    FbxAMatrix* lDeformation = new FbxAMatrix[pSize];
    
    double lATimeSpan = pA->GetTimeSpan().GetSecondDouble();
    double lBTimeSpan = pB->GetTimeSpan().GetSecondDouble();

    FbxTime lATime, lBTime;
    lATime.SetSecondDouble(pTime * lATimeSpan);
    lBTime.SetSecondDouble(pTime * lBTimeSpan);

    FbxAMatrix* lDeformationA = pA->GetClusterDeformation(lATime);
    FbxAMatrix* lDeformationB = pB->GetClusterDeformation(lBTime);

    FbxSkeleton* lSkeleton = pA->mFbxScene->GetSrcObject<FbxSkeleton>();
    if (lSkeleton == nullptr)
    {
        printf("lSkeleton is nullptr\n");
        return lDeformation;
    }

    FbxSkin* lSkin = FbxCast<FbxSkin>(pA->mFbxMesh->GetDeformer(0, FbxDeformer::EDeformerType::eSkin));
    if (lSkin == nullptr)
    {
        printf("lSkin is nullptr\n");
        return lDeformation;
    }

    //printf("%d %d,\n", lSkin->GetSkinningType(), lSkin->GetCluster(0)->GetLinkMode());
    //rigid, and totalOne

    int lClusterCount = lSkin->GetClusterCount();
    for (int lClusterIndex = 0; lClusterIndex < lClusterCount; ++lClusterIndex)
    {
        FbxCluster* lCluster = lSkin->GetCluster(lClusterIndex);

        int lCount = lCluster->GetControlPointIndicesCount();
        int* lIndices = lCluster->GetControlPointIndices();
        double* lWeights = lCluster->GetControlPointWeights();
        
        FbxAMatrix lMA = lDeformationA[lClusterIndex];
        FbxAMatrix lMB = lDeformationB[lClusterIndex];

        FbxVector4 lT = (lMA.GetT() * (1.0f - pAlpha)) + (lMB.GetT() * pAlpha);
        FbxQuaternion lQ = lMA.GetQ().Slerp(lMB.GetQ(), pAlpha);
        FbxVector4 lS = (lMA.GetS() * (1.0f - pAlpha)) + (lMB.GetS() * pAlpha);

        FbxAMatrix lM;
        lM.SetTQS(lT, lQ, lS);

        for (int i = 0; i < lCount; ++i)
        {
            lDeformation[lIndices[i]] = lM;
        }

    }
    /*
    for (int lVertexCount = 0; lVertexCount < size; ++lVertexCount)
    {
        FbxVector4 lT = (pA[lVertexCount].GetT() * (1.0f - pAlpha)) + (pB[lVertexCount].GetT() * pAlpha);
        FbxQuaternion lQ = pA[lVertexCount].GetQ().Slerp(pB[lVertexCount].GetQ(), pAlpha);
        FbxVector4 lS = (pA[lVertexCount].GetS() * (1.0f - pAlpha)) + (pB[lVertexCount].GetS() * pAlpha);

        lNewDeformations[lVertexCount].SetTQS(lT, lQ, lS);
    }
    */
    return lDeformation;
}

FbxAMatrix AnimationLayer::EvaluateBone(const char* pNodeName,FbxTime pTime)
{
    FbxNode* lNode = FbxCast<FbxNode>(mFbxScene->FindSrcObject(FbxCriteria::ObjectType(FbxNode::ClassId), pNodeName));

    return lNode->EvaluateGlobalTransform(pTime);
}

void AnimationLayer::Tick(float pDeltaTime)
{
    if (mEnd > mStart)
    {
        FbxTime lDelta;

        lDelta.SetSecondDouble(pDeltaTime);

        mCurrentTime += lDelta;

        if (mCurrentTime > mEnd)
        {
            mCurrentTime = mStart;
        }
    }
}

FbxTime AnimationLayer::GetTimeSpan() const
{
    return mEnd - mStart;
}

/*
*********************************************************************************
* Character
*********************************************************************************
*/

Character::Character()
{
    mFbxManager = nullptr;
    mFbxIOSettings = nullptr;
    mCurrentTime = 0;
    mStart = 0;
    mEnd = 1;
}

void Character::AddAnimationLayer(const char* pAnimationPath,const char* pName)
{
    mAnimationLayerPaths.push_back({ pAnimationPath,pName });
}

void Character::Initialize(const char* pSkeletalMeshPath)
{
    InitializeFbxSdkObject();

    LoadSkeletal(pSkeletalMeshPath);

    LoadAnimationLayer();    
}

void Character::InitializeFbxSdkObject()
{
    mFbxManager = FbxManager::Create();

    mFbxIOSettings = FbxIOSettings::Create(mFbxManager, IOSROOT);
    mFbxManager->SetIOSettings(mFbxIOSettings);
}

void Character::LoadSkeletal(const char* pSkeletalMeshPath)
{
    mSkeletal = make_shared<Skeletal>(pSkeletalMeshPath);
    mSkeletal->Load(mFbxManager);
}

void Character::LoadAnimationLayer()
{
    for (auto lPath : mAnimationLayerPaths)
    {
        shared_ptr<AnimationLayer> lAnimLayer = make_shared<AnimationLayer>(lPath.first,mSkeletal->GetControlPointCount(),mSkeletal->GetMesh(),mSkeletal->GetClusterCount());
        lAnimLayer->Load(mFbxManager);
        mAnimationLayers[string(lPath.second)] = lAnimLayer;
    }
}

void Character::Draw()
{
    Engine::mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Engine::mCommandList->SetGraphicsRootSignature(Engine::mRootSignatures["Default"].Get());
    Engine::mCommandList->SetPipelineState(Engine::mPSOs["Default"].Get());
    Engine::mCommandList->SetGraphicsRootConstantBufferView(0,
        Engine::mResourceManager->GetResource(Engine::mFrames[Engine::mCurrentFrame]->mObjConstantBufferIdx)->GetGPUVirtualAddress()
        + mSceneNodeIndex * Engine::mResourceManager->CalculateAlignment(sizeof(obj), 256));
    Engine::mCommandList->SetGraphicsRootDescriptorTable(2, Engine::mDescriptorManager->GetGpuHandle(
        Engine::mTextureManager->GetTextureIndex("stone"), DescType::SRV));

    mSkeletal->Draw();
}

void Character::Upload()
{
    mSkeletal->Upload();
}

void Character::VertexUpload()
{
    mSkeletal->VertexUpload();
}

void Character::Update()
{
    mCurrentTime += Engine::mTimer.GetDeltaTime();
    
    if (mCurrentTime > mEnd)
    {
        mCurrentTime = mStart;
    }

    UpdateAnimation();

    SceneNode::Update();
}

void Character::UpdateAnimation()
{
    //test
    //mAnimationLayers["RunF"]->Tick(Engine::mTimer.GetDeltaTime());
    //mAnimationLayers["WalkF"]->Tick(Engine::mTimer.GetDeltaTime());

    //current time도 blend해야한다. (움직임의 sync 맞추기 위해서)

    //FbxAMatrix* lRunDeformation = mAnimationLayers["RunF"]->GetDeformation();
    //FbxAMatrix* lWalkDeformation = mAnimationLayers["WalkF"]->GetDeformation();

    FbxAMatrix* lDeformation = AnimationLayer::Blend(mAnimationLayers["WalkF"].get(), mAnimationLayers["RunF"].get(), 0.5f, mSkeletal->GetControlPointCount(),mCurrentTime);

    mSkeletal->Deform(lDeformation);

    delete[] lDeformation;

    VertexUpload();
    //test end
}