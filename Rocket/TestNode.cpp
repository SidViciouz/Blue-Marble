#include "TestNode.h"
#include "Engine.h"

TestNode::TestNode()
{
    filename = "Model/AnimMan.FBX";

    sdkManager = FbxManager::Create();
    ios = FbxIOSettings::Create(sdkManager, IOSROOT);
    sdkManager->SetIOSettings(ios);
    FbxImporter* lImporter = FbxImporter::Create(sdkManager, "");
    if (!lImporter->Initialize(filename, -1, sdkManager->GetIOSettings()))
    {
        printf("Call to FbxImporter::Initialize() failed.\n");
        printf("Error returned: %s\n\n", lImporter->GetStatus().GetErrorString());
        exit(-1);
    }
    scene1 = FbxScene::Create(sdkManager, "myScene");
    lImporter->Import(scene1);
    lImporter->Destroy();

    Print(scene1->GetRootNode(), 0);

    LoadMaterialData(scene1);
    LoadVertexData(scene1);

    PlayStart(1);

}

void TestNode::Draw()
{

    Engine::mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Engine::mCommandList->SetGraphicsRootSignature(Engine::mRootSignatures["Default"].Get());
    Engine::mCommandList->SetPipelineState(Engine::mPSOs["Default"].Get());
    Engine::mCommandList->SetGraphicsRootConstantBufferView(0,
        Engine::mResourceManager->GetResource(Engine::mFrames[Engine::mCurrentFrame]->mObjConstantBufferIdx)->GetGPUVirtualAddress()
        + mSceneNodeIndex * Engine::mResourceManager->CalculateAlignment(sizeof(obj), 256));
    Engine::mCommandList->SetGraphicsRootDescriptorTable(2, Engine::mDescriptorManager->GetGpuHandle(
        Engine::mTextureManager->GetTextureIndex("stone"), DescType::SRV));

    Engine::mCommandList->IASetVertexBuffers(0, 1, GetVertexBufferView());
    Engine::mCommandList->IASetIndexBuffer(GetIndexBufferView());
    Engine::mCommandList->DrawIndexedInstanced(mIndex.size(), 1, 0, 0, 0);

    SceneNode::Draw();

}

void TestNode::Update()
{
    if (mIsPlayed)
        TimerTick();

    FbxPose* lPose = nullptr;
    if (mPoseIndex != -1)
        lPose = mCurrentScene->GetPose(mPoseIndex);

    FbxAMatrix lDummyGlobalPosition;

    DrawNodeRecursive(mCurrentScene->GetRootNode(), mCurrentTime, mCurrentAnimLayer, lDummyGlobalPosition, lPose);

    SceneNode::Update();
}

void TestNode::Print(FbxNode* pObj, int pTabs)
{
    FbxAMatrix lTransform = pObj->EvaluateGlobalTransform(mCurrentTime);
    /*
    printf("%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n",
        lTransform[0][0], lTransform[0][1], lTransform[0][2], lTransform[0][3],
        lTransform[1][0], lTransform[1][1], lTransform[1][2], lTransform[1][3],
        lTransform[2][0], lTransform[2][1], lTransform[2][2], lTransform[2][3],
        lTransform[3][0], lTransform[3][1], lTransform[3][2], lTransform[3][3]);
     */   
    
    if (pObj->GetNodeAttributeCount())
    {
        FbxNodeAttribute* lAtt = pObj->GetNodeAttribute();

        FbxNodeAttribute::EType lType = lAtt->GetAttributeType();
        
        for(int i=0; i< pTabs; ++i)
            printf("\t");
        printf("%s\t", lAtt->GetName());
        PrintAttributeType(lType);
        
        if (lType == FbxNodeAttribute::EType::eMesh)
        {   
            //PrintMesh(pObj->GetMesh(),pTabs);
        }
    }

    int numChild = pObj->GetChildCount();
    for (int i = 0; i < numChild; ++i)
    {
        FbxNode* child = pObj->GetChild(i);
        Print(child, pTabs + 1);
    }
    
}

D3D12_VERTEX_BUFFER_VIEW* TestNode::GetVertexBufferView()
{
    mVertexBufferView.BufferLocation = Engine::mResourceManager->GetResource(mVertexBuffer)->GetGPUVirtualAddress();
    mVertexBufferView.StrideInBytes = sizeof(Vertex);
    mVertexBufferView.SizeInBytes = sizeof(Vertex) * mVertex.size();

    return &mVertexBufferView;
}

D3D12_INDEX_BUFFER_VIEW* TestNode::GetIndexBufferView()
{
    mIndexBufferView.BufferLocation = Engine::mResourceManager->GetResource(mIndexBuffer)->GetGPUVirtualAddress();
    mIndexBufferView.Format = DXGI_FORMAT_R16_UINT;
    mIndexBufferView.SizeInBytes = sizeof(uint16_t) * mIndex.size();

    return &mIndexBufferView;
}

void TestNode::LoadMaterialData(FbxScene* pScene)
{

    const int lMaterialCount = pScene->GetMaterialCount();

    for (int i = 0; i < lMaterialCount; ++i)
    {
        FbxSurfaceMaterial* lMaterial = pScene->GetMaterial(i);
        
        unsigned int lEmissiveTextureName;
        const FbxDouble3 lEmissive = GetMaterialProperty(lMaterial,
            FbxSurfaceMaterial::sEmissive, FbxSurfaceMaterial::sEmissiveFactor, lEmissiveTextureName);
        printf("emissive %f %f %f, %u\n", lEmissive[0], lEmissive[1], lEmissive[2], lEmissiveTextureName);

        unsigned int lAmbientTextureName;
        const FbxDouble3 lAmbient = GetMaterialProperty(lMaterial,
            FbxSurfaceMaterial::sAmbient, FbxSurfaceMaterial::sAmbientFactor, lAmbientTextureName);
        printf("ambient  %f %f %f, %u\n", lAmbient[0], lAmbient[1], lAmbient[2], lAmbientTextureName);

        unsigned int lDiffuseTextureName;
        const FbxDouble3 lDiffuse = GetMaterialProperty(lMaterial,
            FbxSurfaceMaterial::sDiffuse, FbxSurfaceMaterial::sDiffuseFactor, lDiffuseTextureName);
        printf("diffuse  %f %f %f, %u\n", lDiffuse[0], lDiffuse[1], lDiffuse[2], lDiffuseTextureName);

        unsigned int lSpecularTextureName;
        const FbxDouble3 lSpecular = GetMaterialProperty(lMaterial,
            FbxSurfaceMaterial::sSpecular, FbxSurfaceMaterial::sSpecularFactor, lSpecularTextureName);
        printf("specular  %f %f %f, %u\n", lSpecular[0], lSpecular[1], lSpecular[2], lSpecularTextureName);
    }

}

FbxDouble3 TestNode::GetMaterialProperty(const FbxSurfaceMaterial* pMaterial,
    const char* pPropertyName, const char* pFactorPropertyName, unsigned int& pTextureName)
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

    if (lProperty.IsValid())
    {
        const int lTextureCount = lProperty.GetSrcObjectCount<FbxFileTexture>();
        if (lTextureCount)
        {
            const FbxFileTexture* lTexture = lProperty.GetSrcObject<FbxFileTexture>();
            if (lTexture && lTexture->GetUserDataPtr())
            {
                pTextureName = *(static_cast<int*>(lTexture->GetUserDataPtr()));
            }
        }
    }

    return lResult;
}

void TestNode::LoadVertexData(FbxScene* pScene)
{
    FbxMesh* mesh = FbxCast<FbxMesh>(pScene->GetSrcObject(FbxCriteria::ObjectType(FbxMesh::ClassId)));

    FbxVector4* controlPoints = mesh->GetControlPoints();

    FbxArray<FbxVector4> normals;
    mesh->GetPolygonVertexNormals(normals);

    const char* uvSetName = NULL;
    FbxStringList lUVNames;
    mesh->GetUVSetNames(lUVNames);
    if (lUVNames.GetCount())
    {
        uvSetName = lUVNames[0];
    }


    FbxGeometryElementUV* uvElement = mesh->GetElementUV(0);
    FbxGeometryElement::EMappingMode lUVMappingMode = uvElement->GetMappingMode();

    FbxArray<FbxVector2> uvs;
    mesh->GetPolygonVertexUVs(uvSetName, uvs);

    int triangle[6] = { 0,1,2,0,2,3 };

    if (lUVMappingMode != FbxGeometryElement::eByControlPoint)
    {
        mAllByControlPoint = false;
    }

    if (mAllByControlPoint)
    {
        const FbxGeometryElementNormal* lNormalElement = mesh->GetElementNormal(0);
        const FbxGeometryElementUV* lUVElement = mesh->GetElementUV(0);
        FbxVector4 normal;
        FbxVector2 uv;

        for (int i = 0; i < mesh->GetControlPointsCount(); ++i)
        {
            Vertex vertex;

            FbxVector4 pos = controlPoints[i];
            vertex.position = { static_cast<float>(pos[0]), static_cast<float>(pos[1]), static_cast<float>(pos[2]) };

            int lNormalIndex = i;
            if (lNormalElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
            {
                lNormalIndex = lNormalElement->GetIndexArray().GetAt(i);
            }
            normal = lNormalElement->GetDirectArray().GetAt(lNormalIndex);
            vertex.normal = { static_cast<float>(normal[0]),static_cast<float>(normal[1]), static_cast<float>(normal[2]) };

            int lUVIndex = i;
            if (lUVElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
            {
                lUVIndex = lUVElement->GetIndexArray().GetAt(i);
            }
            uv = lUVElement->GetDirectArray().GetAt(lUVIndex);
            vertex.uv = { static_cast<float>(uv[0]),static_cast<float>(1.0f - uv[1]) };

            mVertex.push_back(vertex);
        }

        int polygonCount = mesh->GetPolygonCount();
        for (int i = 0; i < polygonCount; ++i)
        {
            int vertexCount = mesh->GetPolygonSize(i);
            if (vertexCount == 3)
            {
                for (int j = 0; j < 3; ++j)
                {
                    mIndex.push_back(mesh->GetPolygonVertex(i, j));
                }
            }
            else if (vertexCount == 4)
            {
                for (int j = 0; j < 6; ++j)
                    mIndex.push_back(mesh->GetPolygonVertex(i, triangle[j]));
            }
        }
    }
    else
    {
        int lVertexCount = 0;
        int polygonCount = mesh->GetPolygonCount();
        for (int i = 0; i < polygonCount; ++i)
        {
            int vertexCount = mesh->GetPolygonSize(i);
            if (vertexCount == 3)
            {
                for (int j = 0; j < 3; ++j)
                {
                    int lControlPointIndex = mesh->GetPolygonVertex(i, j);

                    Vertex vertex;

                    FbxVector4 pos = controlPoints[lControlPointIndex];
                    vertex.position = { static_cast<float>(pos[0]), static_cast<float>(pos[1]), static_cast<float>(pos[2]) };

                    FbxVector4 normal;
                    mesh->GetPolygonVertexNormal(i, j, normal);
                    vertex.normal = { static_cast<float>(normal[0]), static_cast<float>(normal[1]), static_cast<float>(normal[2]) };

                    bool lUnmappedUV;
                    FbxVector2 uv;
                    mesh->GetPolygonVertexUV(i, j, uvSetName, uv, lUnmappedUV);
                    vertex.uv = { static_cast<float>(uv[0]),1.0f - static_cast<float>(uv[1]) };

                    mVertex.push_back(vertex);

                    mIndex.push_back(lVertexCount++);
                }
            }
            else if (vertexCount == 4)
            {
                for (int j = 0; j < 6; ++j)
                {
                    int lControlPointIndex = mesh->GetPolygonVertex(i, triangle[j]);

                    Vertex vertex;

                    FbxVector4 pos = controlPoints[lControlPointIndex];
                    vertex.position = { static_cast<float>(pos[0]), static_cast<float>(pos[1]), static_cast<float>(pos[2]) };

                    FbxVector4 normal;
                    mesh->GetPolygonVertexNormal(i, triangle[j], normal);
                    vertex.normal = { static_cast<float>(normal[0]), static_cast<float>(normal[1]), static_cast<float>(normal[2]) };

                    bool lUnmappedUV;
                    FbxVector2 uv;
                    mesh->GetPolygonVertexUV(i, triangle[j], uvSetName, uv, lUnmappedUV);
                    vertex.uv = { static_cast<float>(uv[0]),1.0f - static_cast<float>(uv[1]) };

                    mVertex.push_back(vertex);

                    mIndex.push_back(lVertexCount++);
                }
            }
        }
    }

    mVertexBuffer = Engine::mResourceManager->CreateUploadBuffer(sizeof(Vertex) * mVertex.size());
    mIndexBuffer = Engine::mResourceManager->CreateUploadBuffer(sizeof(uint16_t) * mIndex.size());

    Engine::mResourceManager->Upload(mVertexBuffer, mVertex.data(), sizeof(Vertex) * mVertex.size(), 0);
    Engine::mResourceManager->Upload(mIndexBuffer, mIndex.data(), sizeof(uint16_t)* mIndex.size(), 0);
    
}

bool TestNode::SetCurrentAnimStack(FbxScene* pScene, int pIndex)
{
    mCacheStart = FBXSDK_TIME_INFINITE;
    mCacheStop = FBXSDK_TIME_MINUS_INFINITE;
    mFrameTime.SetTime(0, 0, 0, 1, 0, pScene->GetGlobalSettings().GetTimeMode());
    pScene->FillAnimStackNameArray(mAnimStackNameArray);

    const int lAnimStackCount = mAnimStackNameArray.GetCount();
    if (!lAnimStackCount || pIndex >= lAnimStackCount)
        return false;

    FbxAnimStack* lCurrentAnimationStack = pScene->FindMember<FbxAnimStack>(mAnimStackNameArray[pIndex]->Buffer());
    if (lCurrentAnimationStack == nullptr)
        return false;

    //여기서 첫번째로 애니메이션 스택에 연결된 레이어가 베이스 레이어라고 가정한다.
    mCurrentAnimLayer = lCurrentAnimationStack->GetMember<FbxAnimLayer>();
    pScene->SetCurrentAnimationStack(lCurrentAnimationStack);

    FbxTakeInfo* lCurrentTakeInfo = pScene->GetTakeInfo(*(mAnimStackNameArray[pIndex]));

    if (lCurrentTakeInfo)
    {
        mStart = lCurrentTakeInfo->mLocalTimeSpan.GetStart();
        mStop = lCurrentTakeInfo->mLocalTimeSpan.GetStop();
    }
    else
    {
        FbxTimeSpan lTimeLineTimeSpan;
        pScene->GetGlobalSettings().GetTimelineDefaultTimeSpan(lTimeLineTimeSpan);

        mStart = lTimeLineTimeSpan.GetStart();
        mStop = lTimeLineTimeSpan.GetStop();
    }

    if (mCacheStart < mStart)
        mStart = mCacheStart;

    if (mCacheStop > mStop)
        mStop = mCacheStop;

    mCurrentTime = mStart;

    return true;
}

void TestNode::DrawNodeRecursive(FbxNode* pNode, FbxTime& pTime, FbxAnimLayer* pAnimLayer,
    FbxAMatrix& pParentGlobalPositoin, FbxPose* pPose)
{
    FbxAMatrix lGlobalPosition = GetGlobalPosition(pNode, pTime, pPose, &pParentGlobalPositoin);
    if (pNode->GetNodeAttribute())
    {
        FbxAMatrix lGeometryOffset = GetGeometry(pNode);
        FbxAMatrix lGlobalOffPosition = lGlobalPosition * lGeometryOffset;
        DrawNode(pNode, pTime, pAnimLayer, pParentGlobalPositoin, lGlobalOffPosition, pPose);
    }

    const int lChildCount = pNode->GetChildCount();
    for (int lChildIndex = 0; lChildIndex < lChildCount; ++lChildIndex)
    {
        DrawNodeRecursive(pNode->GetChild(lChildIndex), pTime, pAnimLayer, lGlobalPosition, pPose);
    }

}

FbxAMatrix TestNode::GetGlobalPosition(FbxNode* pNode, const FbxTime& pTime, FbxPose* pPose,
    FbxAMatrix* pParentGlobalPosition)
{
    FbxAMatrix lGlobalPosition;
    bool lPositionFound = false;

    if (pPose)
    {
        int lNodeIndex = pPose->Find(pNode);

        if (lNodeIndex > -1)
        {
            if (pPose->IsBindPose() || !pPose->IsLocalMatrix(lNodeIndex))
            {
                lGlobalPosition = GetPoseMatrix(pPose, lNodeIndex);
            }
            else
            {
                FbxAMatrix lParentGlobalPostion;

                if (pParentGlobalPosition)
                {
                    lParentGlobalPostion = *pParentGlobalPosition;
                }
                else
                {
                    if (pNode->GetParent())
                    {
                        lParentGlobalPostion = GetGlobalPosition(pNode->GetParent(), pTime, pPose);
                    }
                }
                FbxAMatrix lLocalPosition = GetPoseMatrix(pPose, lNodeIndex);
                lGlobalPosition = lParentGlobalPostion * lLocalPosition;
            }

            lPositionFound = true;
        }
    }

    if (!lPositionFound)
    {
        lGlobalPosition = pNode->EvaluateGlobalTransform(pTime);
    }

    return lGlobalPosition;
}

FbxAMatrix TestNode::GetPoseMatrix(FbxPose* pPose, int pNodeIndex)
{
    FbxAMatrix lPoseMatrix;
    FbxMatrix lMatrix = pPose->GetMatrix(pNodeIndex);

    memcpy((double*)lPoseMatrix, (double*)lMatrix, sizeof(lMatrix.mData));

    return lPoseMatrix;
}

void TestNode::TimerTick()
{
    if (mStop > mStart)
    {

        mFrameTime.SetSecondDouble(Engine::mTimer.GetDeltaTime());
        mCurrentTime += mFrameTime;

        if (mCurrentTime > mStop)
        {
            mCurrentTime = mStart;
        }
    }
}

FbxAMatrix TestNode::GetGeometry(FbxNode* pNode)
{
    const FbxVector4 lT = pNode->GetGeometricTranslation(FbxNode::eSourcePivot);
    const FbxVector4 lR = pNode->GetGeometricRotation(FbxNode::eSourcePivot);
    const FbxVector4 lS = pNode->GetGeometricScaling(FbxNode::eSourcePivot);

    return FbxAMatrix(lT, lR, lS);
}

void TestNode::DrawNode(FbxNode* pNode, FbxTime& pTime, FbxAnimLayer* pAnimLayer, FbxAMatrix& pParentGlobalPosition,
    FbxAMatrix& pGlobalPosition, FbxPose* pPose)
{
    FbxNodeAttribute* lNodeAttribute = pNode->GetNodeAttribute();
    if (lNodeAttribute)
    {
        if (lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eSkeleton)
        {

        }
        else if (lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh)
        {
            DrawMesh(pNode, pTime, pAnimLayer, pGlobalPosition, pPose);
        }
    }
}
void TestNode::DrawMesh(FbxNode* pNode, FbxTime& pTime, FbxAnimLayer* pAnimLayer,
    FbxAMatrix& pGlobalPosition, FbxPose* pPose)
{
    FbxMesh* lMesh = pNode->GetMesh();
    const int lVertexCount = lMesh->GetControlPointsCount();

    if (lVertexCount == 0)
        return;

    //lMesh->GetUserDataPtr();

    const bool lHasVertexCache = lMesh->GetDeformerCount(FbxDeformer::eVertexCache) &&
        (static_cast<FbxVertexCacheDeformer*>(lMesh->GetDeformer(0, FbxDeformer::eVertexCache)))->Active;
    const bool lHasShape = lMesh->GetShapeCount();
    const bool lHasSkin = lMesh->GetDeformerCount(FbxDeformer::eSkin) > 0;
    const bool lHasDeformation = lHasVertexCache || lHasShape || lHasSkin;

    //FbxVector4* lVertexArray = NULL;

    if (!mMeshCache && lHasDeformation)
    {

        lVertexArray = new FbxVector4[lVertexCount];
        memcpy(lVertexArray, lMesh->GetControlPoints(), lVertexCount * sizeof(FbxVector4));
    }

    if (lHasDeformation)
    {
        if (lHasVertexCache)
        {
            //read vertex cache data
            ReadVertexCacheData(lMesh, pTime, lVertexArray);
        }
        else
        {
            if (lHasShape)
            {
                //compute shape deformation
                printf("compute shape deformation\n");
                //ComputeShapeDeformation(lMesh, pTime, pAnimLayer, lVertexArray);
            }

            const int lSkinCount = lMesh->GetDeformerCount(FbxDeformer::eSkin);
            int lClusterCount = 0;
            for (int lSkinIndex = 0; lSkinIndex < lSkinCount; ++lSkinIndex)
            {
                lClusterCount += ((FbxSkin*)(lMesh->GetDeformer(lSkinIndex, FbxDeformer::eSkin)))->GetClusterCount();
            }
            if (lClusterCount)
            {
                //compute skin deformation
                ComputeSkinDeformation(pGlobalPosition, lMesh, pTime, lVertexArray, pPose);
            }
        }

        //update vertex position
        UpdateVertexPosition(lMesh, lVertexArray);
    }

}

void TestNode::ReadVertexCacheData(FbxMesh* pMesh, FbxTime& pTime, FbxVector4* pVertexArray)
{
    FbxVertexCacheDeformer* lDeformer = static_cast<FbxVertexCacheDeformer*>(pMesh->GetDeformer(0, FbxDeformer::eVertexCache));
    FbxCache* lCache = lDeformer->GetCache();
    int lChannelIndex = -1;
    unsigned int lVertexCount = (unsigned int)pMesh->GetControlPointsCount();
    bool lReadSucceed = false;
    double* lReadBuf = new double[3 * lVertexCount];

    if (lCache->GetCacheFileFormat() == FbxCache::eMayaCache)
    {
        if ((lChannelIndex = lCache->GetChannelCount() - 1) > -1)
        {
            lReadSucceed = lCache->Read(lChannelIndex, pTime, lReadBuf, lVertexCount, 0);
        }
    }
    else
    {
        lReadSucceed = lCache->Read((unsigned int)pTime.GetFrameCount(), lReadBuf, lVertexCount);
    }

    if (lReadSucceed)
    {
        unsigned int lReadBufIndex = 0;

        while (lReadBufIndex < 3 * lVertexCount)
        {
            pVertexArray[lReadBufIndex / 3].mData[0] = lReadBuf[lReadBufIndex]; lReadBufIndex++;
            pVertexArray[lReadBufIndex / 3].mData[1] = lReadBuf[lReadBufIndex]; lReadBufIndex++;
            pVertexArray[lReadBufIndex / 3].mData[2] = lReadBuf[lReadBufIndex]; lReadBufIndex++;
        }
    }

    delete[] lReadBuf;
}


void TestNode::UpdateVertexPosition(const FbxMesh* pMesh, const FbxVector4* pVertices)
{

    // Convert to the same sequence with data in GPU.
    int lVertexCount = 0;

    if (mAllByControlPoint)
    {
        lVertexCount = pMesh->GetControlPointsCount();
        for (int lIndex = 0; lIndex < lVertexCount; ++lIndex)
        {
            mVertex[lIndex].position.x = static_cast<float>(pVertices[lIndex][0]);
            mVertex[lIndex].position.y = static_cast<float>(pVertices[lIndex][1]);
            mVertex[lIndex].position.z = static_cast<float>(pVertices[lIndex][2]);
        }
    }

    else
    {
        const int lPolygonCount = pMesh->GetPolygonCount();
        int triangle[6] = { 0,1,2,0,2,3 };


        int lVertexCount = 0;
        for (int lPolygonIndex = 0; lPolygonIndex < lPolygonCount; ++lPolygonIndex)
        {
            int vertexCount = pMesh->GetPolygonSize(lPolygonIndex);

            if (vertexCount == 3)
            {
                for (int lVerticeIndex = 0; lVerticeIndex < 3; ++lVerticeIndex)
                {
                    const int lControlPointIndex = pMesh->GetPolygonVertex(lPolygonIndex, lVerticeIndex);
                    mVertex[lVertexCount].position.x = static_cast<float>(pVertices[lControlPointIndex][0]);
                    mVertex[lVertexCount].position.y = static_cast<float>(pVertices[lControlPointIndex][1]);
                    mVertex[lVertexCount].position.z = static_cast<float>(pVertices[lControlPointIndex][2]);
                    ++lVertexCount;
                }
            }
            else if (vertexCount == 4)
            {
                for (int lVerticeIndex = 0; lVerticeIndex < 6; ++lVerticeIndex)
                {
                    const int lControlPointIndex = pMesh->GetPolygonVertex(lPolygonIndex, triangle[lVerticeIndex]);
                    mVertex[lVertexCount].position.x = static_cast<float>(pVertices[lControlPointIndex][0]);
                    mVertex[lVertexCount].position.y = static_cast<float>(pVertices[lControlPointIndex][1]);
                    mVertex[lVertexCount].position.z = static_cast<float>(pVertices[lControlPointIndex][2]);
                    ++lVertexCount;
                }
            }
        }
    }
    // Transfer into GPU.

    Engine::mResourceManager->Upload(mVertexBuffer, mVertex.data(), sizeof(Vertex) * mVertex.size(), 0);
    Engine::mResourceManager->Upload(mIndexBuffer, mIndex.data(), sizeof(uint16_t) * mIndex.size(), 0);

}

void TestNode::ComputeSkinDeformation(FbxAMatrix& pGlobalPosition, FbxMesh* pMesh, FbxTime& pTime, FbxVector4* pVertexArray, FbxPose* pPose)
{

    FbxSkin* lSkinDeformer = (FbxSkin*)pMesh->GetDeformer(0, FbxDeformer::eSkin);
    FbxSkin::EType lSkinningType = lSkinDeformer->GetSkinningType();

    if (lSkinningType == FbxSkin::eLinear || lSkinningType == FbxSkin::eRigid)
    {
        ComputeLinearDeformation(pGlobalPosition, pMesh, pTime, pVertexArray, pPose);
    }
    else if (lSkinningType == FbxSkin::eDualQuaternion)
    {
        ComputeDualQuaternionDeformation(pGlobalPosition, pMesh, pTime, pVertexArray, pPose);
    }
    else if (lSkinningType == FbxSkin::eBlend)
    {
        int lVertexCount = pMesh->GetControlPointsCount();

        FbxVector4* lVertexArrayLinear = new FbxVector4[lVertexCount];
        memcpy(lVertexArrayLinear, pMesh->GetControlPoints(), lVertexCount * sizeof(FbxVector4));

        FbxVector4* lVertexArrayDQ = new FbxVector4[lVertexCount];
        memcpy(lVertexArrayDQ, pMesh->GetControlPoints(), lVertexCount * sizeof(FbxVector4));

        ComputeLinearDeformation(pGlobalPosition, pMesh, pTime, lVertexArrayLinear, pPose);
        ComputeDualQuaternionDeformation(pGlobalPosition, pMesh, pTime, lVertexArrayDQ, pPose);

        // To blend the skinning according to the blend weights
        // Final vertex = DQSVertex * blend weight + LinearVertex * (1- blend weight)
        // DQSVertex: vertex that is deformed by dual quaternion skinning method;
        // LinearVertex: vertex that is deformed by classic linear skinning method;
        int lBlendWeightsCount = lSkinDeformer->GetControlPointIndicesCount();
        for (int lBWIndex = 0; lBWIndex < lBlendWeightsCount; ++lBWIndex)
        {
            double lBlendWeight = lSkinDeformer->GetControlPointBlendWeights()[lBWIndex];
            pVertexArray[lBWIndex] = lVertexArrayDQ[lBWIndex] * lBlendWeight + lVertexArrayLinear[lBWIndex] * (1 - lBlendWeight);
        }
    }

}


void TestNode::ComputeLinearDeformation(FbxAMatrix& pGlobalPosition, FbxMesh* pMesh, FbxTime& pTime, FbxVector4* pVertexArray, FbxPose* pPose)
{

    // All the links must have the same link mode.
    FbxCluster::ELinkMode lClusterMode = ((FbxSkin*)pMesh->GetDeformer(0, FbxDeformer::eSkin))->GetCluster(0)->GetLinkMode();

    int lVertexCount = pMesh->GetControlPointsCount();
    FbxAMatrix* lClusterDeformation = new FbxAMatrix[lVertexCount];
    memset(lClusterDeformation, 0, lVertexCount * sizeof(FbxAMatrix));

    double* lClusterWeight = new double[lVertexCount];
    memset(lClusterWeight, 0, lVertexCount * sizeof(double));

    if (lClusterMode == FbxCluster::eAdditive)
    {
        for (int i = 0; i < lVertexCount; ++i)
        {
            lClusterDeformation[i].SetIdentity();
        }
    }

    // For all skins and all clusters, accumulate their deformation and weight
    // on each vertices and store them in lClusterDeformation and lClusterWeight.
    int lSkinCount = pMesh->GetDeformerCount(FbxDeformer::eSkin);
    for (int lSkinIndex = 0; lSkinIndex < lSkinCount; ++lSkinIndex)
    {
        FbxSkin* lSkinDeformer = (FbxSkin*)pMesh->GetDeformer(lSkinIndex, FbxDeformer::eSkin);

        int lClusterCount = lSkinDeformer->GetClusterCount();
        for (int lClusterIndex = 0; lClusterIndex < lClusterCount; ++lClusterIndex)
        {
            FbxCluster* lCluster = lSkinDeformer->GetCluster(lClusterIndex);
            if (!lCluster->GetLink())
                continue;

            FbxAMatrix lVertexTransformMatrix;
            ComputeClusterDeformation(pGlobalPosition, pMesh, lCluster, lVertexTransformMatrix, pTime, pPose);

            int lVertexIndexCount = lCluster->GetControlPointIndicesCount();
            for (int k = 0; k < lVertexIndexCount; ++k)
            {
                int lIndex = lCluster->GetControlPointIndices()[k];

                // Sometimes, the mesh can have less points than at the time of the skinning
                // because a smooth operator was active when skinning but has been deactivated during export.
                if (lIndex >= lVertexCount)
                    continue;

                double lWeight = lCluster->GetControlPointWeights()[k];

                if (lWeight == 0.0)
                {
                    continue;
                }

                // Compute the influence of the link on the vertex.
                FbxAMatrix lInfluence = lVertexTransformMatrix;
                MatrixScale(lInfluence, lWeight);

                if (lClusterMode == FbxCluster::eAdditive)
                {
                    // Multiply with the product of the deformations on the vertex.
                    MatrixAddToDiagonal(lInfluence, 1.0 - lWeight);
                    lClusterDeformation[lIndex] = lInfluence * lClusterDeformation[lIndex];

                    // Set the link to 1.0 just to know this vertex is influenced by a link.
                    lClusterWeight[lIndex] = 1.0;
                }
                else // lLinkMode == FbxCluster::eNormalize || lLinkMode == FbxCluster::eTotalOne
                {
                    // Add to the sum of the deformations on the vertex.
                    MatrixAdd(lClusterDeformation[lIndex], lInfluence);

                    // Add to the sum of weights to either normalize or complete the vertex.
                    lClusterWeight[lIndex] += lWeight;
                }
            }//For each vertex			
        }//lClusterCount
    }

    //Actually deform each vertices here by information stored in lClusterDeformation and lClusterWeight
    for (int i = 0; i < lVertexCount; i++)
    {
        FbxVector4 lSrcVertex = pVertexArray[i];
        FbxVector4& lDstVertex = pVertexArray[i];
        double lWeight = lClusterWeight[i];

        // Deform the vertex if there was at least a link with an influence on the vertex,
        if (lWeight != 0.0)
        {
            lDstVertex = lClusterDeformation[i].MultT(lSrcVertex);
            if (lClusterMode == FbxCluster::eNormalize)
            {
                // In the normalized link mode, a vertex is always totally influenced by the links. 
                lDstVertex /= lWeight;
            }
            else if (lClusterMode == FbxCluster::eTotalOne)
            {
                // In the total 1 link mode, a vertex can be partially influenced by the links. 
                lSrcVertex *= (1.0 - lWeight);
                lDstVertex += lSrcVertex;
            }
        }
    }

    delete[] lClusterDeformation;
    delete[] lClusterWeight;

}


void TestNode::ComputeClusterDeformation(FbxAMatrix& pGlobalPosition,
    FbxMesh* pMesh, FbxCluster* pCluster, FbxAMatrix& pVertexTransformMatrix, FbxTime pTime, FbxPose* pPose)
{

    FbxCluster::ELinkMode lClusterMode = pCluster->GetLinkMode();

    FbxAMatrix lReferenceGlobalInitPosition;
    FbxAMatrix lReferenceGlobalCurrentPosition;
    FbxAMatrix lAssociateGlobalInitPosition;
    FbxAMatrix lAssociateGlobalCurrentPosition;
    FbxAMatrix lClusterGlobalInitPosition;
    FbxAMatrix lClusterGlobalCurrentPosition;

    FbxAMatrix lReferenceGeometry;
    FbxAMatrix lAssociateGeometry;
    FbxAMatrix lClusterGeometry;

    FbxAMatrix lClusterRelativeInitPosition;
    FbxAMatrix lClusterRelativeCurrentPositionInverse;

    if (lClusterMode == FbxCluster::eAdditive && pCluster->GetAssociateModel())
    {
        pCluster->GetTransformAssociateModelMatrix(lAssociateGlobalInitPosition);
        // Geometric transform of the model
        lAssociateGeometry = GetGeometry(pCluster->GetAssociateModel());
        lAssociateGlobalInitPosition *= lAssociateGeometry;
        lAssociateGlobalCurrentPosition = GetGlobalPosition(pCluster->GetAssociateModel(), pTime, pPose);

        pCluster->GetTransformMatrix(lReferenceGlobalInitPosition);
        // Multiply lReferenceGlobalInitPosition by Geometric Transformation
        lReferenceGeometry = GetGeometry(pMesh->GetNode());
        lReferenceGlobalInitPosition *= lReferenceGeometry;
        lReferenceGlobalCurrentPosition = pGlobalPosition;

        // Get the link initial global position and the link current global position.
        pCluster->GetTransformLinkMatrix(lClusterGlobalInitPosition);
        // Multiply lClusterGlobalInitPosition by Geometric Transformation
        lClusterGeometry = GetGeometry(pCluster->GetLink());
        lClusterGlobalInitPosition *= lClusterGeometry;
        lClusterGlobalCurrentPosition = GetGlobalPosition(pCluster->GetLink(), pTime, pPose);

        // Compute the shift of the link relative to the reference.
        //ModelM-1 * AssoM * AssoGX-1 * LinkGX * LinkM-1*ModelM
        pVertexTransformMatrix = lReferenceGlobalInitPosition.Inverse() * lAssociateGlobalInitPosition * lAssociateGlobalCurrentPosition.Inverse() *
            lClusterGlobalCurrentPosition * lClusterGlobalInitPosition.Inverse() * lReferenceGlobalInitPosition;
    }
    else
    {
        /*
        FbxAMatrix parent;
        pCluster->GetTransformParentMatrix(parent);
        printf("[%s]\n", pCluster->GetName());
        for(int i=0;i <pCluster->GetDstObjectCount(); ++i)
            printf("%s ", pCluster->GetDstObject(i)->GetName());
        printf("\n");

        printf("%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n\n",
            parent[0][0], parent[0][1], parent[0][2], parent[0][3],
            parent[1][0], parent[1][1], parent[1][2], parent[1][3],
            parent[2][0], parent[2][1], parent[2][2], parent[2][3],
            parent[3][0], parent[3][1], parent[3][2], parent[3][3]);
            */


        pCluster->GetTransformMatrix(lReferenceGlobalInitPosition);
        lReferenceGlobalCurrentPosition = pGlobalPosition;
        // Multiply lReferenceGlobalInitPosition by Geometric Transformation
        lReferenceGeometry = GetGeometry(pMesh->GetNode());
        lReferenceGlobalInitPosition *= lReferenceGeometry;
        /*
        if (strcmp(pCluster->GetName(), "Bone") == 0)
        {
            printf("%s\n\n", pCluster->GetName());
            printf("%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n\n",
                lReferenceGlobalInitPosition[0][0], lReferenceGlobalInitPosition[0][1], lReferenceGlobalInitPosition[0][2], lReferenceGlobalInitPosition[0][3],
                lReferenceGlobalInitPosition[1][0], lReferenceGlobalInitPosition[1][1], lReferenceGlobalInitPosition[1][2], lReferenceGlobalInitPosition[1][3],
                lReferenceGlobalInitPosition[2][0], lReferenceGlobalInitPosition[2][1], lReferenceGlobalInitPosition[2][2], lReferenceGlobalInitPosition[2][3],
                lReferenceGlobalInitPosition[3][0], lReferenceGlobalInitPosition[3][1], lReferenceGlobalInitPosition[3][2], lReferenceGlobalInitPosition[3][3]);

            printf("%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n\n",
                lReferenceGlobalCurrentPosition[0][0], lReferenceGlobalCurrentPosition[0][1], lReferenceGlobalCurrentPosition[0][2], lReferenceGlobalCurrentPosition[0][3],
                lReferenceGlobalCurrentPosition[1][0], lReferenceGlobalCurrentPosition[1][1], lReferenceGlobalCurrentPosition[1][2], lReferenceGlobalCurrentPosition[1][3],
                lReferenceGlobalCurrentPosition[2][0], lReferenceGlobalCurrentPosition[2][1], lReferenceGlobalCurrentPosition[2][2], lReferenceGlobalCurrentPosition[2][3],
                lReferenceGlobalCurrentPosition[3][0], lReferenceGlobalCurrentPosition[3][1], lReferenceGlobalCurrentPosition[3][2], lReferenceGlobalCurrentPosition[3][3]);
        }
        */
        // Get the link initial global position and the link current global position.
        pCluster->GetTransformLinkMatrix(lClusterGlobalInitPosition);
        lClusterGlobalCurrentPosition = GetGlobalPosition(pCluster->GetLink(), pTime, pPose);
        /*
        if (strcmp(pCluster->GetName(), "Bone") == 0)
        {
            printf("%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n\n",
                lClusterGlobalInitPosition[0][0], lClusterGlobalInitPosition[0][1], lClusterGlobalInitPosition[0][2], lClusterGlobalInitPosition[0][3],
                lClusterGlobalInitPosition[1][0], lClusterGlobalInitPosition[1][1], lClusterGlobalInitPosition[1][2], lClusterGlobalInitPosition[1][3],
                lClusterGlobalInitPosition[2][0], lClusterGlobalInitPosition[2][1], lClusterGlobalInitPosition[2][2], lClusterGlobalInitPosition[2][3],
                lClusterGlobalInitPosition[3][0], lClusterGlobalInitPosition[3][1], lClusterGlobalInitPosition[3][2], lClusterGlobalInitPosition[3][3]);

            printf("%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n\n",
                lClusterGlobalCurrentPosition[0][0], lClusterGlobalCurrentPosition[0][1], lClusterGlobalCurrentPosition[0][2], lClusterGlobalCurrentPosition[0][3],
                lClusterGlobalCurrentPosition[1][0], lClusterGlobalCurrentPosition[1][1], lClusterGlobalCurrentPosition[1][2], lClusterGlobalCurrentPosition[1][3],
                lClusterGlobalCurrentPosition[2][0], lClusterGlobalCurrentPosition[2][1], lClusterGlobalCurrentPosition[2][2], lClusterGlobalCurrentPosition[2][3],
                lClusterGlobalCurrentPosition[3][0], lClusterGlobalCurrentPosition[3][1], lClusterGlobalCurrentPosition[3][2], lClusterGlobalCurrentPosition[3][3]);
        }
        */
        // Compute the initial position of the link relative to the reference.
        //lClusterRelativeInitPosition = lClusterGlobalInitPosition.Inverse() * lReferenceGlobalInitPosition;
        
        // Compute the current position of the link relative to the reference.
        //lClusterRelativeCurrentPositionInverse = lReferenceGlobalCurrentPosition.Inverse() * lClusterGlobalCurrentPosition;

        FbxAMatrix RI = lClusterGlobalInitPosition * lReferenceGlobalInitPosition.Inverse();
        FbxAMatrix RC = lClusterGlobalCurrentPosition * lReferenceGlobalCurrentPosition.Inverse();

        pVertexTransformMatrix = RC * RI.Inverse();

        // Compute the shift of the link relative to the reference.
        //pVertexTransformMatrix = lClusterRelativeCurrentPositionInverse * lClusterRelativeInitPosition;
        /*
        if (strcmp(pCluster->GetName(), "Bone") == 0)
        {
            printf("%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n\n",
                lClusterRelativeInitPosition[0][0], lClusterRelativeInitPosition[0][1], lClusterRelativeInitPosition[0][2], lClusterRelativeInitPosition[0][3],
                lClusterRelativeInitPosition[1][0], lClusterRelativeInitPosition[1][1], lClusterRelativeInitPosition[1][2], lClusterRelativeInitPosition[1][3],
                lClusterRelativeInitPosition[2][0], lClusterRelativeInitPosition[2][1], lClusterRelativeInitPosition[2][2], lClusterRelativeInitPosition[2][3],
                lClusterRelativeInitPosition[3][0], lClusterRelativeInitPosition[3][1], lClusterRelativeInitPosition[3][2], lClusterRelativeInitPosition[3][3]);

            printf("%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n\n",
                lClusterRelativeCurrentPositionInverse[0][0], lClusterRelativeCurrentPositionInverse[0][1], lClusterRelativeCurrentPositionInverse[0][2], lClusterRelativeCurrentPositionInverse[0][3],
                lClusterRelativeCurrentPositionInverse[1][0], lClusterRelativeCurrentPositionInverse[1][1], lClusterRelativeCurrentPositionInverse[1][2], lClusterRelativeCurrentPositionInverse[1][3],
                lClusterRelativeCurrentPositionInverse[2][0], lClusterRelativeCurrentPositionInverse[2][1], lClusterRelativeCurrentPositionInverse[2][2], lClusterRelativeCurrentPositionInverse[2][3],
                lClusterRelativeCurrentPositionInverse[3][0], lClusterRelativeCurrentPositionInverse[3][1], lClusterRelativeCurrentPositionInverse[3][2], lClusterRelativeCurrentPositionInverse[3][3]);

            printf("%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n\n----------------------------------\n\n",
                pVertexTransformMatrix[0][0], pVertexTransformMatrix[0][1], pVertexTransformMatrix[0][2], pVertexTransformMatrix[0][3],
                pVertexTransformMatrix[1][0], pVertexTransformMatrix[1][1], pVertexTransformMatrix[1][2], pVertexTransformMatrix[1][3],
                pVertexTransformMatrix[2][0], pVertexTransformMatrix[2][1], pVertexTransformMatrix[2][2], pVertexTransformMatrix[2][3],
                pVertexTransformMatrix[3][0], pVertexTransformMatrix[3][1], pVertexTransformMatrix[3][2], pVertexTransformMatrix[3][3]);
        }
        */
    }

}



void TestNode::MatrixScale(FbxAMatrix& pMatrix, double pValue)
{

    int i, j;

    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 4; j++)
        {
            pMatrix[i][j] *= pValue;
        }
    }

}

void TestNode::MatrixAddToDiagonal(FbxAMatrix& pMatrix, double pValue)
{
    pMatrix[0][0] += pValue;
    pMatrix[1][1] += pValue;
    pMatrix[2][2] += pValue;
    pMatrix[3][3] += pValue;
}


void TestNode::MatrixAdd(FbxAMatrix& pDstMatrix, FbxAMatrix& pSrcMatrix)
{

    int i, j;

    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 4; j++)
        {
            pDstMatrix[i][j] += pSrcMatrix[i][j];
        }
    }

}


void TestNode::ComputeDualQuaternionDeformation(FbxAMatrix& pGlobalPosition,
    FbxMesh* pMesh, FbxTime& pTime, FbxVector4* pVertexArray, FbxPose* pPose)
{

    // All the links must have the same link mode.
    FbxCluster::ELinkMode lClusterMode = ((FbxSkin*)pMesh->GetDeformer(0, FbxDeformer::eSkin))->GetCluster(0)->GetLinkMode();

    int lVertexCount = pMesh->GetControlPointsCount();
    int lSkinCount = pMesh->GetDeformerCount(FbxDeformer::eSkin);

    FbxDualQuaternion* lDQClusterDeformation = new FbxDualQuaternion[lVertexCount];
    memset(lDQClusterDeformation, 0, lVertexCount * sizeof(FbxDualQuaternion));

    double* lClusterWeight = new double[lVertexCount];
    memset(lClusterWeight, 0, lVertexCount * sizeof(double));

    // For all skins and all clusters, accumulate their deformation and weight
    // on each vertices and store them in lClusterDeformation and lClusterWeight.
    for (int lSkinIndex = 0; lSkinIndex < lSkinCount; ++lSkinIndex)
    {
        FbxSkin* lSkinDeformer = (FbxSkin*)pMesh->GetDeformer(lSkinIndex, FbxDeformer::eSkin);
        int lClusterCount = lSkinDeformer->GetClusterCount();
        for (int lClusterIndex = 0; lClusterIndex < lClusterCount; ++lClusterIndex)
        {
            FbxCluster* lCluster = lSkinDeformer->GetCluster(lClusterIndex);
            if (!lCluster->GetLink())
                continue;

            FbxAMatrix lVertexTransformMatrix;
            ComputeClusterDeformation(pGlobalPosition, pMesh, lCluster, lVertexTransformMatrix, pTime, pPose);

            FbxQuaternion lQ = lVertexTransformMatrix.GetQ();
            FbxVector4 lT = lVertexTransformMatrix.GetT();
            FbxDualQuaternion lDualQuaternion(lQ, lT);

            int lVertexIndexCount = lCluster->GetControlPointIndicesCount();
            for (int k = 0; k < lVertexIndexCount; ++k)
            {
                int lIndex = lCluster->GetControlPointIndices()[k];

                // Sometimes, the mesh can have less points than at the time of the skinning
                // because a smooth operator was active when skinning but has been deactivated during export.
                if (lIndex >= lVertexCount)
                    continue;

                double lWeight = lCluster->GetControlPointWeights()[k];

                if (lWeight == 0.0)
                    continue;

                // Compute the influence of the link on the vertex.
                FbxDualQuaternion lInfluence = lDualQuaternion * lWeight;
                if (lClusterMode == FbxCluster::eAdditive)
                {
                    // Simply influenced by the dual quaternion.
                    lDQClusterDeformation[lIndex] = lInfluence;

                    // Set the link to 1.0 just to know this vertex is influenced by a link.
                    lClusterWeight[lIndex] = 1.0;
                }
                else // lLinkMode == FbxCluster::eNormalize || lLinkMode == FbxCluster::eTotalOne
                {
                    if (lClusterIndex == 0)
                    {
                        lDQClusterDeformation[lIndex] = lInfluence;
                    }
                    else
                    {
                        // Add to the sum of the deformations on the vertex.
                        // Make sure the deformation is accumulated in the same rotation direction. 
                        // Use dot product to judge the sign.
                        double lSign = lDQClusterDeformation[lIndex].GetFirstQuaternion().DotProduct(lDualQuaternion.GetFirstQuaternion());
                        if (lSign >= 0.0)
                        {
                            lDQClusterDeformation[lIndex] += lInfluence;
                        }
                        else
                        {
                            lDQClusterDeformation[lIndex] -= lInfluence;
                        }
                    }
                    // Add to the sum of weights to either normalize or complete the vertex.
                    lClusterWeight[lIndex] += lWeight;
                }
            }//For each vertex
        }//lClusterCount
    }

    //Actually deform each vertices here by information stored in lClusterDeformation and lClusterWeight
    for (int i = 0; i < lVertexCount; i++)
    {
        FbxVector4 lSrcVertex = pVertexArray[i];
        FbxVector4& lDstVertex = pVertexArray[i];
        double lWeightSum = lClusterWeight[i];

        // Deform the vertex if there was at least a link with an influence on the vertex,
        if (lWeightSum != 0.0)
        {
            lDQClusterDeformation[i].Normalize();
            lDstVertex = lDQClusterDeformation[i].Deform(lDstVertex);

            if (lClusterMode == FbxCluster::eNormalize)
            {
                // In the normalized link mode, a vertex is always totally influenced by the links. 
                lDstVertex /= lWeightSum;
            }
            else if (lClusterMode == FbxCluster::eTotalOne)
            {
                // In the total 1 link mode, a vertex can be partially influenced by the links. 
                lSrcVertex *= (1.0 - lWeightSum);
                lDstVertex += lSrcVertex;
            }
        }
    }

    delete[] lDQClusterDeformation;
    delete[] lClusterWeight;

}

void TestNode::PlayStart(int index)
{
    if (index == 1)
    {
        SetCurrentAnimStack(scene1, 0);

        mCurrentScene = scene1;
    }
    else if (index == 2)
    {
        SetCurrentAnimStack(scene1, 0);

        mCurrentScene = scene1;
    }

    mIsPlayed = true;
}

void TestNode::PlayEnd()
{
    mIsPlayed = false;
}


void TestNode::PrintAttributeType(FbxNodeAttribute::EType pType)
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

    case FbxNodeAttribute::EType::eLine :
        printf("Etype : eLine\n");
        break;
    }
}

void TestNode::PrintMesh(FbxMesh* pMesh, int pTabs)
{
    
    if (pMesh->GetDeformerCount(FbxDeformer::eSkin))
    {
        FbxSkin* lSkin = FbxCast<FbxSkin>(pMesh->GetDeformer(0, FbxDeformer::eSkin));

        int lClusterCount = lSkin->GetClusterCount();
        /*
        printf("-----------------------------------\n");
        for (int i = 0; i < lClusterCount; ++i)
        {
            FbxCluster* lCluster = lSkin->GetCluster(i);

            FbxAMatrix lM1;
            FbxAMatrix lM2;
            lCluster->GetTransformMatrix(lM1);
            lCluster->GetTransformLinkMatrix(lM2);

            if (strcmp(lCluster->GetName(), "Bone") == 0)
            {
                FbxNode* lBone = lCluster->GetLink();
                FbxVector4 lTrans = lBone->EvaluateGlobalTransform(mCurrentTime).GetT();
                
                FbxAnimCurveNode* lAnimCurveNode = lBone->LclTranslation.GetCurveNode(mCurrentAnimLayer,true);

                
                if (lAnimCurveNode == nullptr)
                    printf("lAnimCurveNode is null\n");
                else
                {
                    int lChannelCount = lAnimCurveNode->GetChannelsCount();
                    //key(FbxAnimCurveKey) 추가
                    FbxAnimCurve* lCurve = lAnimCurveNode->GetCurve(2);
                    FbxTime lTime;
                    int lKeyIndex = 0;
                    lTime.SetSecondDouble(2.0f);
                    
                    lCurve->KeyModifyBegin();
                    lKeyIndex = lCurve->KeyAdd(lTime);
                    lCurve->KeySet(lKeyIndex, lTime, 0.01f, FbxAnimCurveDef::eInterpolationLinear);
                    lCurve->KeyModifyEnd();

                    for (int lChannelIdx = 0; lChannelIdx < lChannelCount; ++lChannelIdx)
                    {
                        printf("%s\n", lAnimCurveNode->GetChannelName(lChannelIdx).Buffer());
                        
                        FbxAnimCurve* lCurve =  lAnimCurveNode->GetCurve(lChannelIdx);

                        int lKeyCount = lCurve->KeyGetCount();
                        for (int lKeyIdx = 0; lKeyIdx < lKeyCount; ++lKeyIdx)
                        {
                            printf("[%f %f] ", lCurve->KeyGetTime(lKeyIdx).GetSecondDouble(),lCurve->KeyGetValue(lKeyIdx));
                        }
                        printf("\n");
                    }
                }
                //printf("%f %f %f %f\n", lTrans[0], lTrans[1], lTrans[2], lTrans[3]);
            }


            
            printf("%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n\n",
                lM1[0][0], lM1[0][1], lM1[0][2], lM1[0][3],
                lM1[1][0], lM1[1][1], lM1[1][2], lM1[1][3],
                lM1[2][0], lM1[2][1], lM1[2][2], lM1[2][3],
                lM1[3][0], lM1[3][1], lM1[3][2], lM1[3][3]
                );

            printf("%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n\n\n\n",
                lM2[0][0], lM2[0][1], lM2[0][2], lM2[0][3],
                lM2[1][0], lM2[1][1], lM2[1][2], lM2[1][3],
                lM2[2][0], lM2[2][1], lM2[2][2], lM2[2][3],
                lM2[3][0], lM2[3][1], lM2[3][2], lM2[3][3]
            );
                
        }
        printf("-----------------------------------\n");
        */
    }
}