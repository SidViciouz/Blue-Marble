#include "AnimationNode.h"
#include "Engine.h"

AnimatedNode::AnimatedNode()
{
    filename = "Model/standing_idle.fbx";
    filename2 = "Model/medium_running.fbx";

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

    sdkManager = FbxManager::Create();
    ios = FbxIOSettings::Create(sdkManager, IOSROOT);
    sdkManager->SetIOSettings(ios);
    lImporter = FbxImporter::Create(sdkManager, "");
    if (!lImporter->Initialize(filename2, -1, sdkManager->GetIOSettings()))
    {
        printf("Call to FbxImporter::Initialize() failed.\n");
        printf("Error returned: %s\n\n", lImporter->GetStatus().GetErrorString());
        exit(-1);
    }
    scene2 = FbxScene::Create(sdkManager, "myScene");
    lImporter->Import(scene2);
    lImporter->Destroy();

    LoadMaterialData(scene1);
    LoadVertexData(scene1);

    PlayStart(1);
}

void AnimatedNode::Draw()
{
    
    Engine::mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Engine::mCommandList->SetGraphicsRootSignature(Engine::mRootSignatures["Default"].Get());
    Engine::mCommandList->SetPipelineState(Engine::mPSOs["Default"].Get());
    Engine::mCommandList->SetGraphicsRootConstantBufferView(0,
        Engine::mResourceManager->GetResource(Engine::mFrames[Engine::mCurrentFrame]->mObjConstantBufferIdx)->GetGPUVirtualAddress()
        + mSceneNodeIndex * Engine::mResourceManager->CalculateAlignment(sizeof(obj), 256));
    Engine::mCommandList->SetGraphicsRootDescriptorTable(2, Engine::mDescriptorManager->GetGpuHandle(
        Engine::mTextureManager->GetTextureIndex("peasant"), DescType::SRV));

    Engine::mCommandList->IASetVertexBuffers(0, 1, GetVertexBufferView());
    Engine::mCommandList->IASetIndexBuffer(GetIndexBufferView());
    Engine::mCommandList->DrawIndexedInstanced(mIndex.size(), 1, 0, 0, 0);

    SceneNode::Draw();
    
}

void AnimatedNode::Update()
{
    if(mIsPlayed)
        TimerTick();

    FbxPose* lPose = nullptr;
    if (mPoseIndex != -1)
        lPose = mCurrentScene->GetPose(mPoseIndex);

    FbxAMatrix lDummyGlobalPosition;

    DrawNodeRecursive(mCurrentScene->GetRootNode(), mCurrentTime, currentAnimLayer, lDummyGlobalPosition, lPose);

    SceneNode::Update();
}

void AnimatedNode::Print(FbxNode* obj,int tabs)
{
    FbxSkeleton* skeleton = obj->GetSkeleton();
    for(int i=0; i<tabs; ++i)
        printf("\t");
    printf("name : %s | type name : %s\n\n", obj->GetName(), obj->GetTypeName());

    if (skeleton != nullptr)
    {
        for (int i = 0; i < tabs; ++i)
            printf("\t");
        printf("skeleton name : %s\n\n", skeleton->GetName());
    }
    FbxMesh* mesh = obj->GetMesh();
    if (mesh != nullptr)
    {
        for (int i = 0; i < mesh->GetDeformerCount(); ++i)
        {
            FbxDeformer* deformer = mesh->GetDeformer(i);
            for (int i = 0; i < tabs+1; ++i)
                printf("\t");
            printf("[deformer] name : %s, type name : %s\n\n", deformer->GetName(), deformer->GetTypeName());
            FbxSkin* skin = FbxCast<FbxSkin>(deformer);
            if (skin != nullptr)
            {
                int clusterCount = skin->GetClusterCount();
                for (int i = 0; i < clusterCount; ++i)
                {
                    FbxCluster* cluster =  skin->GetCluster(i);

                    for (int i = 0; i < tabs+2; ++i)
                        printf("\t");
                    FbxNode* link = cluster->GetLink();
                    int ctrlPointCount = cluster->GetControlPointIndicesCount();
                    printf("[cluster link] name : %s, type name :%s, control point count : %d\n\n", link->GetName(), link->GetTypeName(),ctrlPointCount);

                    FbxAnimEvaluator* eval = link->GetAnimationEvaluator();
                    FbxTime time1,time2,time3;
                    time1.SetSecondDouble(0);
                    time2.SetSecondDouble(1);
                    time3.SetSecondDouble(1.5f);
                    for (int i = 0; i < tabs + 2; ++i)
                        printf("\t");
                    
                    FbxVector4 before = eval->GetNodeLocalRotation(link, time1);
                    FbxVector4 after = eval->GetNodeLocalRotation(link, time2);
                    FbxVector4 after2 = eval->GetNodeLocalRotation(link, time3);
                    printf("%f %f %f %f -> %f %f %f %f\n",before[0],before[1],before[2],before[3],
                        after[0], after[1], after[2], after[3]);
                    printf("-> %f %f %f %f\n", after2[0], after2[1], after2[2], after2[3]);
                }
            }
        }
    }


    int numChild = obj->GetChildCount();
    for (int i = 0; i < numChild; ++i)
    {
        FbxNode* child = obj->GetChild(i);
        Print(child,tabs+1);
    }
}

D3D12_VERTEX_BUFFER_VIEW* AnimatedNode::GetVertexBufferView()
{
    mVertexBufferView.BufferLocation = Engine::mResourceManager->GetResource(mVertexBuffer)->GetGPUVirtualAddress();
    mVertexBufferView.StrideInBytes = sizeof(Vertex);
    mVertexBufferView.SizeInBytes = sizeof(Vertex) * mVertex.size();

    return &mVertexBufferView;
}

D3D12_INDEX_BUFFER_VIEW* AnimatedNode::GetIndexBufferView()
{
    mIndexBufferView.BufferLocation = Engine::mResourceManager->GetResource(mIndexBuffer)->GetGPUVirtualAddress();
    mIndexBufferView.Format = DXGI_FORMAT_R16_UINT;
    mIndexBufferView.SizeInBytes = sizeof(uint16_t) * mIndex.size();

    return &mIndexBufferView;
}

void AnimatedNode::LoadMaterialData(FbxScene* pScene)
{

    const int lMaterialCount = pScene->GetMaterialCount();

    for (int i = 0; i < lMaterialCount; ++i)
    {
        FbxSurfaceMaterial* lMaterial = pScene->GetMaterial(i);
        
        unsigned int lEmissiveTextureName;
        const FbxDouble3 lEmissive = GetMaterialProperty(lMaterial,
            FbxSurfaceMaterial::sEmissive, FbxSurfaceMaterial::sEmissiveFactor, lEmissiveTextureName);
        
        unsigned int lAmbientTextureName;
        const FbxDouble3 lAmbient = GetMaterialProperty(lMaterial,
            FbxSurfaceMaterial::sAmbient, FbxSurfaceMaterial::sAmbientFactor, lAmbientTextureName);

        unsigned int lDiffuseTextureName;
        const FbxDouble3 lDiffuse = GetMaterialProperty(lMaterial,
            FbxSurfaceMaterial::sDiffuse, FbxSurfaceMaterial::sDiffuseFactor, lDiffuseTextureName);

        unsigned int lSpecularTextureName;
        const FbxDouble3 lSpecular = GetMaterialProperty(lMaterial,
            FbxSurfaceMaterial::sSpecular, FbxSurfaceMaterial::sSpecularFactor, lSpecularTextureName);
    }

}

FbxDouble3 AnimatedNode::GetMaterialProperty(const FbxSurfaceMaterial* pMaterial,
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

void AnimatedNode::LoadVertexData(FbxScene* pScene)
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
                for(int j=0; j<6; ++j)
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
                    int lControlPointIndex = mesh->GetPolygonVertex(i,j);

                    Vertex vertex;

                    FbxVector4 pos = controlPoints[lControlPointIndex];
                    vertex.position = { static_cast<float>(pos[0]), static_cast<float>(pos[1]), static_cast<float>(pos[2]) };
                    
                    FbxVector4 normal;
                    mesh->GetPolygonVertexNormal(i, j, normal);
                    vertex.normal = { static_cast<float>(normal[0]), static_cast<float>(normal[1]), static_cast<float>(normal[2]) };

                    bool lUnmappedUV;
                    FbxVector2 uv;
                    mesh->GetPolygonVertexUV(i, j, uvSetName, uv, lUnmappedUV);
                    vertex.uv = {static_cast<float>(uv[0]),1.0f-static_cast<float>(uv[1]) };
                    
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
                    vertex.uv = {static_cast<float>(uv[0]),1.0f-static_cast<float>(uv[1]) };

                    mVertex.push_back(vertex);

                    mIndex.push_back(lVertexCount++);
                }
            }
        }
    } 

    mVertexBuffer = Engine::mResourceManager->CreateUploadBuffer(sizeof(Vertex) * mVertex.size());
    mIndexBuffer = Engine::mResourceManager->CreateUploadBuffer(sizeof(uint16_t) * mIndex.size());

    Engine::mResourceManager->Upload(mVertexBuffer, mVertex.data(), sizeof(Vertex) * mVertex.size(), 0);
    Engine::mResourceManager->Upload(mIndexBuffer, mIndex.data(), sizeof(uint16_t) * mIndex.size(), 0);

    /*
    FbxNode* root = scene->GetRootNode();
    int numChild = root->GetChildCount();
    for (int i = 0; i < numChild; ++i)
    {
        FbxNode* obj = root->GetChild(i);
        Print(obj, 1);
    }
    */
}

bool AnimatedNode::SetCurrentAnimStack(FbxScene* pScene,int pIndex)
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
    currentAnimLayer = lCurrentAnimationStack->GetMember<FbxAnimLayer>();
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

void AnimatedNode::DrawNodeRecursive(FbxNode* pNode, FbxTime& pTime, FbxAnimLayer* pAnimLayer,
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

FbxAMatrix AnimatedNode::GetGlobalPosition(FbxNode* pNode, const FbxTime& pTime, FbxPose* pPose,
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
                        lParentGlobalPostion = GetGlobalPosition(pNode->GetParent(),pTime,pPose);
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

FbxAMatrix AnimatedNode::GetPoseMatrix(FbxPose* pPose, int pNodeIndex)
{
    FbxAMatrix lPoseMatrix;
    FbxMatrix lMatrix = pPose->GetMatrix(pNodeIndex);

    memcpy((double*)lPoseMatrix, (double*)lMatrix, sizeof(lMatrix.mData));

    return lPoseMatrix;
}

void AnimatedNode::TimerTick()
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

FbxAMatrix AnimatedNode::GetGeometry(FbxNode* pNode)
{
    const FbxVector4 lT = pNode->GetGeometricTranslation(FbxNode::eSourcePivot);
    const FbxVector4 lR = pNode->GetGeometricRotation(FbxNode::eSourcePivot);
    const FbxVector4 lS = pNode->GetGeometricScaling(FbxNode::eSourcePivot);

    return FbxAMatrix(lT, lR, lS);
}

void AnimatedNode::DrawNode(FbxNode* pNode, FbxTime& pTime, FbxAnimLayer* pAnimLayer, FbxAMatrix& pParentGlobalPosition,
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
void AnimatedNode::DrawMesh(FbxNode* pNode, FbxTime& pTime, FbxAnimLayer* pAnimLayer,
    FbxAMatrix& pGlobalPosition, FbxPose* pPose)
{
    FbxMesh* lMesh = pNode->GetMesh();
    const int lVertexCount = lMesh->GetControlPointsCount();

    if (lVertexCount == 0)
        return;


    const bool lHasVertexCache = lMesh->GetDeformerCount(FbxDeformer::eVertexCache) &&
        (static_cast<FbxVertexCacheDeformer*>(lMesh->GetDeformer(0, FbxDeformer::eVertexCache)))->Active;
    const bool lHasShape = lMesh->GetShapeCount();
    const bool lHasSkin = lMesh->GetDeformerCount(FbxDeformer::eSkin) > 0;
    const bool lHasDeformation = lHasVertexCache || lHasShape || lHasSkin;
    

    if (!mMeshCache && lHasDeformation)
    {

        lVertexArray = new FbxVector4[lVertexCount];
        memcpy(lVertexArray, lMesh->GetControlPoints(), lVertexCount * sizeof(FbxVector4));
    }

    if (lHasDeformation)
    {
        if (lHasVertexCache)
        {
            ReadVertexCacheData(lMesh, pTime, lVertexArray);
        }
        else
        {
            if (lHasShape)
            {
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
                ComputeSkinDeformation(pGlobalPosition, lMesh, pTime, lVertexArray, pPose);
            }
        }

        UpdateVertexPosition(lMesh, lVertexArray);
    }
   
}

void AnimatedNode::ReadVertexCacheData(FbxMesh* pMesh, FbxTime& pTime, FbxVector4* pVertexArray)
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
            lReadSucceed = lCache->Read(lChannelIndex, pTime, lReadBuf, lVertexCount,0);
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


void AnimatedNode::UpdateVertexPosition(const FbxMesh* pMesh, const FbxVector4* pVertices)
{
    
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

    Engine::mResourceManager->Upload(mVertexBuffer, mVertex.data(), sizeof(Vertex) * mVertex.size(), 0);
    Engine::mResourceManager->Upload(mIndexBuffer, mIndex.data(), sizeof(uint16_t) * mIndex.size(), 0);
    
}

void AnimatedNode::ComputeSkinDeformation(FbxAMatrix& pGlobalPosition, FbxMesh* pMesh, FbxTime& pTime, FbxVector4* pVertexArray, FbxPose* pPose)
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

        int lBlendWeightsCount = lSkinDeformer->GetControlPointIndicesCount();
        for (int lBWIndex = 0; lBWIndex < lBlendWeightsCount; ++lBWIndex)
        {
            double lBlendWeight = lSkinDeformer->GetControlPointBlendWeights()[lBWIndex];
            pVertexArray[lBWIndex] = lVertexArrayDQ[lBWIndex] * lBlendWeight + lVertexArrayLinear[lBWIndex] * (1 - lBlendWeight);
        }
    }

}


void AnimatedNode::ComputeLinearDeformation(FbxAMatrix& pGlobalPosition, FbxMesh* pMesh, FbxTime& pTime, FbxVector4* pVertexArray, FbxPose* pPose)
{

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

                if (lIndex >= lVertexCount)
                    continue;

                double lWeight = lCluster->GetControlPointWeights()[k];

                if (lWeight == 0.0)
                {
                    continue;
                }

                FbxAMatrix lInfluence = lVertexTransformMatrix;
                MatrixScale(lInfluence, lWeight);

                if (lClusterMode == FbxCluster::eAdditive)
                {
                    MatrixAddToDiagonal(lInfluence, 1.0 - lWeight);
                    lClusterDeformation[lIndex] = lInfluence * lClusterDeformation[lIndex];

                    lClusterWeight[lIndex] = 1.0;
                }
                else
                {
                    MatrixAdd(lClusterDeformation[lIndex], lInfluence);

                    lClusterWeight[lIndex] += lWeight;
                }
            }			
        }
    }

    for (int i = 0; i < lVertexCount; i++)
    {
        FbxVector4 lSrcVertex = pVertexArray[i];
        FbxVector4& lDstVertex = pVertexArray[i];
        double lWeight = lClusterWeight[i];

        if (lWeight != 0.0)
        {
            lDstVertex = lClusterDeformation[i].MultT(lSrcVertex);
            if (lClusterMode == FbxCluster::eNormalize)
            {
                lDstVertex /= lWeight;
            }
            else if (lClusterMode == FbxCluster::eTotalOne)
            {
                lSrcVertex *= (1.0 - lWeight);
                lDstVertex += lSrcVertex;
            }
        }
    }

    delete[] lClusterDeformation;
    delete[] lClusterWeight;

}


void AnimatedNode::ComputeClusterDeformation(FbxAMatrix& pGlobalPosition,
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

        lAssociateGeometry = GetGeometry(pCluster->GetAssociateModel());
        lAssociateGlobalInitPosition *= lAssociateGeometry;
        lAssociateGlobalCurrentPosition = GetGlobalPosition(pCluster->GetAssociateModel(), pTime, pPose);

        pCluster->GetTransformMatrix(lReferenceGlobalInitPosition);

        lReferenceGeometry = GetGeometry(pMesh->GetNode());
        lReferenceGlobalInitPosition *= lReferenceGeometry;
        lReferenceGlobalCurrentPosition = pGlobalPosition;

        pCluster->GetTransformLinkMatrix(lClusterGlobalInitPosition);

        lClusterGeometry = GetGeometry(pCluster->GetLink());
        lClusterGlobalInitPosition *= lClusterGeometry;
        lClusterGlobalCurrentPosition = GetGlobalPosition(pCluster->GetLink(), pTime, pPose);

        pVertexTransformMatrix = lReferenceGlobalInitPosition.Inverse() * lAssociateGlobalInitPosition * lAssociateGlobalCurrentPosition.Inverse() *
            lClusterGlobalCurrentPosition * lClusterGlobalInitPosition.Inverse() * lReferenceGlobalInitPosition;
    }
    else
    {
        pCluster->GetTransformMatrix(lReferenceGlobalInitPosition);
        lReferenceGlobalCurrentPosition = pGlobalPosition;

        lReferenceGeometry = GetGeometry(pMesh->GetNode());
        lReferenceGlobalInitPosition *= lReferenceGeometry;

        pCluster->GetTransformLinkMatrix(lClusterGlobalInitPosition);
        lClusterGlobalCurrentPosition = GetGlobalPosition(pCluster->GetLink(), pTime, pPose);

        lClusterRelativeInitPosition = lClusterGlobalInitPosition.Inverse() * lReferenceGlobalInitPosition;

        lClusterRelativeCurrentPositionInverse = lReferenceGlobalCurrentPosition.Inverse() * lClusterGlobalCurrentPosition;

        pVertexTransformMatrix = lClusterRelativeCurrentPositionInverse * lClusterRelativeInitPosition;
    }

}



void AnimatedNode::MatrixScale(FbxAMatrix& pMatrix, double pValue)
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

void AnimatedNode::MatrixAddToDiagonal(FbxAMatrix& pMatrix, double pValue)
{
    pMatrix[0][0] += pValue;
    pMatrix[1][1] += pValue;
    pMatrix[2][2] += pValue;
    pMatrix[3][3] += pValue;
}


void AnimatedNode::MatrixAdd(FbxAMatrix& pDstMatrix, FbxAMatrix& pSrcMatrix)
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


void AnimatedNode::ComputeDualQuaternionDeformation(FbxAMatrix& pGlobalPosition,
    FbxMesh* pMesh, FbxTime& pTime, FbxVector4* pVertexArray, FbxPose* pPose)
{

    FbxCluster::ELinkMode lClusterMode = ((FbxSkin*)pMesh->GetDeformer(0, FbxDeformer::eSkin))->GetCluster(0)->GetLinkMode();

    int lVertexCount = pMesh->GetControlPointsCount();
    int lSkinCount = pMesh->GetDeformerCount(FbxDeformer::eSkin);

    FbxDualQuaternion* lDQClusterDeformation = new FbxDualQuaternion[lVertexCount];
    memset(lDQClusterDeformation, 0, lVertexCount * sizeof(FbxDualQuaternion));

    double* lClusterWeight = new double[lVertexCount];
    memset(lClusterWeight, 0, lVertexCount * sizeof(double));

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

                if (lIndex >= lVertexCount)
                    continue;

                double lWeight = lCluster->GetControlPointWeights()[k];

                if (lWeight == 0.0)
                    continue;

                FbxDualQuaternion lInfluence = lDualQuaternion * lWeight;
                if (lClusterMode == FbxCluster::eAdditive)
                {
                    lDQClusterDeformation[lIndex] = lInfluence;

                    lClusterWeight[lIndex] = 1.0;
                }
                else
                {
                    if (lClusterIndex == 0)
                    {
                        lDQClusterDeformation[lIndex] = lInfluence;
                    }
                    else
                    {
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
                    lClusterWeight[lIndex] += lWeight;
                }
            }
        }
    }

    for (int i = 0; i < lVertexCount; i++)
    {
        FbxVector4 lSrcVertex = pVertexArray[i];
        FbxVector4& lDstVertex = pVertexArray[i];
        double lWeightSum = lClusterWeight[i];

        if (lWeightSum != 0.0)
        {
            lDQClusterDeformation[i].Normalize();
            lDstVertex = lDQClusterDeformation[i].Deform(lDstVertex);

            if (lClusterMode == FbxCluster::eNormalize)
            {
                lDstVertex /= lWeightSum;
            }
            else if (lClusterMode == FbxCluster::eTotalOne)
            {
                lSrcVertex *= (1.0 - lWeightSum);
                lDstVertex += lSrcVertex;
            }
        }
    }

    delete[] lDQClusterDeformation;
    delete[] lClusterWeight;

}

void AnimatedNode::PlayStart(int index)
{
    if (index == 1)
    {
        SetCurrentAnimStack(scene1, 0);

        mCurrentScene = scene1;
    }
    else if (index == 2)
    {
        SetCurrentAnimStack(scene2, 0);

        mCurrentScene = scene2;
    }

    mIsPlayed = true;
}

void AnimatedNode::PlayEnd()
{
    mIsPlayed = false;
}