#include "Character.h"

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

void SubMesh::AddIndex(int pIndex)
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
                mSubMeshes[lMaterialIndex]->AddIndex(lIndex);

            }

            mSubMeshes[lMaterialIndex]->AddTriangle();
        }
        else if (lPolygonSize == 4)
        {
            for (int i = 0; i < 6; ++i)
            {
                int lIndex = mFbxMesh->GetPolygonVertex(lPolygonIndex, mTriangle[i]);
                mSubMeshes[lMaterialIndex]->AddIndex(lIndex);

            }

            mSubMeshes[lMaterialIndex]->AddTriangle(2);
        }
        else
        {
            printf("lPolygonSize is %d\n", lPolygonSize);
        }

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
* Character
*********************************************************************************
*/

Character::Character()
{
    mFbxManager = nullptr;
    mFbxIOSettings = nullptr;
}

void Character::AddAnimationLayer(const char* pAnimationPath)
{
	mAnimationLayerPaths.push_back(pAnimationPath);
}

void Character::Initialize(const char* pSkeletalMeshPath)
{
    InitializeFbxSdkObject();

    LoadSkeletal(pSkeletalMeshPath);
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



/*
#include "Engine.h"
void Character::Upload()
{
    mVertexBuffer = Engine::mResourceManager->CreateUploadBuffer(sizeof(Vertex) * mVertex.size());
    mIndexBuffer = Engine::mResourceManager->CreateUploadBuffer(sizeof(uint16_t) * mIndex.size());

    Engine::mResourceManager->Upload(mVertexBuffer, mVertex.data(), sizeof(Vertex) * mVertex.size(), 0);
    Engine::mResourceManager->Upload(mIndexBuffer, mIndex.data(), sizeof(uint16_t) * mIndex.size(), 0);
}

D3D12_VERTEX_BUFFER_VIEW* Character::GetVertexBufferView()
{
    mVertexBufferView.BufferLocation = Engine::mResourceManager->GetResource(mVertexBuffer)->GetGPUVirtualAddress();
    mVertexBufferView.StrideInBytes = sizeof(Vertex);
    mVertexBufferView.SizeInBytes = sizeof(Vertex) * mVertex.size();

    return &mVertexBufferView;
}

D3D12_INDEX_BUFFER_VIEW* Character::GetIndexBufferView()
{
    mIndexBufferView.BufferLocation = Engine::mResourceManager->GetResource(mIndexBuffer)->GetGPUVirtualAddress();
    mIndexBufferView.Format = DXGI_FORMAT_R16_UINT;
    mIndexBufferView.SizeInBytes = sizeof(uint16_t) * mIndex.size();

    return &mIndexBufferView;
}
*/