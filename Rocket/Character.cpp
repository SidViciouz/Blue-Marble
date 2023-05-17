#include "Character.h"

/*
*********************************************************************************
* Material
*********************************************************************************
*/



/*
*********************************************************************************
* SkeletalMesh
*********************************************************************************
*/

SkeletalMesh::SkeletalMesh()
{

}

void SkeletalMesh::Load()
{

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

}

void Skeletal::LoadSkeletalMesh()
{
    mSkeletalMesh = make_shared<SkeletalMesh>();
    mSkeletalMesh->Load();
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