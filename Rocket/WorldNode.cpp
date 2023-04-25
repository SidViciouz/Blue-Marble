#include "WorldNode.h"
#include "Engine.h"
#include "json/json.h"
#include <fstream>
#include <iostream>

WorldNode::WorldNode(string name)
	: MeshNode(name)
{
	mCharacter = make_shared<MeshNode>("box");
	mCharacter->SetRelativePosition(35.0f, 0.0f, 0.0f);
	mCharacter->SetTextureName("stone");
	AddChild(mCharacter);
	
	mBorderTextureIdx = Engine::mResourceManager->CreateTexture2D(3600, 1800,
		DXGI_FORMAT_R8_UNORM,D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	
	mBorderTextureUavIdx = Engine::mDescriptorManager->CreateUav(
		Engine::mResourceManager->GetResource(mBorderTextureIdx),
		DXGI_FORMAT_R8_UNORM,
		D3D12_UAV_DIMENSION_TEXTURE2D
	);
	
	mUploadBufferIdx = Engine::mResourceManager->CreateUploadBuffer(
		Engine::mResourceManager->CalculateAlignment(3600,256)* 1800);
	
	Json::Value root;
	std::ifstream config_doc("../Data/world-administrative-boundaries.json", std::ifstream::binary);
	config_doc >> root;
	string str;

	for (int i = 0; i < root.size(); ++i)
	{
		Json::Value field = root[i]["geo_shape"]["geometry"]["coordinates"];

		CountryInfo info;

		info.geo.x = root[i]["geo_point_2d"]["lon"].asDouble();
		info.geo.y = root[i]["geo_point_2d"]["lat"].asDouble();

		for (auto it = field.begin(); it != field.end(); it++)
		{
			vector<struct Point> points;
			for (auto jt = it->begin(); jt != it->end(); jt++)
			{
				if (jt->size() == 2)
				{
					int i = 0;
					float coord[2];
					bool isCoord = false;
					for (auto ht = jt->begin(); ht != jt->end(); ht++)
					{
						isCoord = true;
						Json::StyledWriter writer;
						coord[i++] = ht->asDouble();
					}
					if (isCoord)
					{
						points.push_back({ coord[1] ,coord[0] });
						data[1800 - (((int)(coord[1] * 10) + 900))][((int)(coord[0] * 10) + 1800)] = 1;
					}

					continue;
				}

				for (auto kt = jt->begin(); kt != jt->end(); kt++)
				{
					int i = 0;
					float coord[2];
					bool isCoord = false;
					for (auto ht = kt->begin(); ht != kt->end(); ht++)
					{
						isCoord = true;
						Json::StyledWriter writer;
						coord[i++] = ht->asDouble();
					}
					if (isCoord)
					{
						points.push_back({ coord[1] ,coord[0] });
						data[1800-(((int)(coord[1]*10)+900))][((int)(coord[0]*10)+1800)] = 1;
					}
				}
			}
			if (points.size() != 0)
				info.points.push_back(points);
		}
		mCountryInfos[root[i]["name"].asString()] = info;
	}
	
	/*
	for (auto country : mCountryInfos)
	{
		//cout << country.first << "\n";
		int i = 0;
		for (auto points : country.second.points)
		{
			//printf("%d\n", i);
			for (auto point : points)
			{
				//printf("%f %f\n", point.x, point.y);
			}
			++i;
		}
	}
	*/
	Engine::mResourceManager->UploadTexture2D(mUploadBufferIdx, data, 3600, 1800, 0, 0);
	Engine::mResourceManager->CopyUploadToTexture(mUploadBufferIdx, mBorderTextureIdx, 3600, 1800, 1, DXGI_FORMAT_R8_UNORM, 1);
}

void WorldNode::Draw()
{
	if (!mActivated)
		return;

	Engine::mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
	Engine::mCommandList->SetGraphicsRootSignature(Engine::mRootSignatures["earth"].Get());
	Engine::mCommandList->SetPipelineState(Engine::mPSOs["earth"].Get());
	Engine::mCommandList->SetGraphicsRootConstantBufferView(0,
		Engine::mResourceManager->GetResource(Engine::mFrames[Engine::mCurrentFrame]->mObjConstantBufferIdx)->GetGPUVirtualAddress()
		+ mSceneNodeIndex * Engine::mResourceManager->CalculateAlignment(sizeof(obj), 256));

	Engine::mCommandList->SetGraphicsRoot32BitConstant(4, 1, 0);

	Engine::mCommandList->SetGraphicsRootDescriptorTable(2, Engine::mDescriptorManager->GetGpuHandle(
		Engine::mAllScenes[Engine::mCurrentSceneName]->mShadowMap->GetTextureSrvIdx(), DescType::SRV));

	Engine::mCommandList->SetGraphicsRootDescriptorTable(3, Engine::mDescriptorManager->GetGpuHandle(
		Engine::mTextureManager->GetTextureIndex(mTextureName), DescType::SRV));

	Engine::mCommandList->SetGraphicsRootDescriptorTable(5, Engine::mDescriptorManager->GetGpuHandle(
		Engine::mTextureManager->GetTextureIndex("world"), DescType::SRV));

	Engine::mCommandList->SetGraphicsRootDescriptorTable(6, Engine::mDescriptorManager->GetGpuHandle(
		mBorderTextureUavIdx, DescType::SRV));

	Engine::mMeshManager->Draw(mMeshName);

	SceneNode::Draw();
}

void WorldNode::Update()
{
	/*
	* 여기에 추가로 필요한 것들을 작성한다.
	*/

	UpdateCharacter();

	if (!mActivated)
		return;

	SceneNode::Update();
}

void WorldNode::MoveCharacter(const XMFLOAT3& pos)
{
	isMoving = true;

	XMFLOAT3 curPos = mCharacter->GetAccumulatedPosition().Get();
	
	mMoveInfo.totalFrame = 60;
	mMoveInfo.curFrame = 0;
	mMoveInfo.radius = GetScale().x;

	XMFLOAT3 center = mAccumulatedPosition.Get();
	Vector3 v1(curPos.x - center.x, curPos.y - center.y, curPos.z - center.z);
	Vector3 v2(pos.x - center.x, pos.y - center.y, pos.z - center.z);

	v1 = v1.normalize();
	v2 = v2.normalize();

	mMoveInfo.axis = (v1 ^ v2).normalize().v;
	mMoveInfo.angle = acos(v1 * v2)/ mMoveInfo.totalFrame;
}

void WorldNode::PickCountry(const XMFLOAT3& pos)
{
	XMVECTOR xmPos = XMLoadFloat3(&pos);
	XMVECTOR xmQuat = XMLoadFloat4(&mAccumulatedQuaternion.Get());
	xmQuat = XMQuaternionInverse(xmQuat);

	//world coordinate -> spherical coordinate
	XMVECTOR xmCenter = XMLoadFloat3(&mAccumulatedPosition.Get());
	xmPos = xmPos - xmCenter;

	xmPos = XMVector3Rotate(xmPos, xmQuat);
	xmPos = XMVector3Normalize(xmPos);

	//spherical coordinate -> geographic coordinate
	XMVECTOR xmProjXZ = XMVectorSetY(xmPos, 0);
	xmProjXZ = XMVector3Normalize(xmProjXZ);

	XMVECTOR xmFront = XMVectorSet(0, 0, 1, 0);

	XMVECTOR xmRight = XMVectorSet(1, 0, 0, 0);

	XMVECTOR xmUp = XMVectorSet(0, 1, 0, 0);

	XMFLOAT3 up,ppos;
	XMStoreFloat3(&up, xmUp);
	XMStoreFloat3(&ppos, xmPos);
	
	float latitude = 90.0f - XMConvertToDegrees(acos(XMVectorGetX(XMVector3Dot(xmPos, xmUp))));
	float longitude = XMConvertToDegrees(acos(XMVectorGetX(XMVector3Dot(xmProjXZ, xmFront))));

	if (XMVectorGetX(XMVector3Dot(xmProjXZ, xmRight)) < 0)
		longitude *= -1.0f;
	
	latitude *= -1.0f;
	longitude += 269;
	if (longitude > 360)
		longitude -= 360;

	//geographics coordinate -> uv coordinate
	float u = (longitude + 180) / 360.0f;
	float v = (latitude + 90) / 180.0f;

	printf("latitude : %f, longitude : %f\n", latitude, longitude);
	//printf("u : %f, v : %f\n", u, v);
	for (auto country : mCountryInfos)
	{
		struct Point p = country.second.geo;
		float dx = longitude - p.x;
		float dy = latitude - p.y;

		dx = dx * dx;
		dy = dy * dy;

		if (dx + dy < 25)
		{
			cout << country.first << "\n";
		}
	}

	cout << "\n";
}

void WorldNode::UpdateCharacter()
{
	if (!isMoving)
		return;

	if (mMoveInfo.totalFrame < mMoveInfo.curFrame)
	{
		isMoving = false;
		return;
	}

	++mMoveInfo.curFrame;

	XMVECTOR quat = XMQuaternionRotationAxis(XMLoadFloat3(&mMoveInfo.axis), mMoveInfo.angle);

	XMFLOAT3 curPos = mCharacter->GetAccumulatedPosition().Get();
	XMFLOAT3 center = mAccumulatedPosition.Get();

	XMVECTOR xmCurPos = XMLoadFloat3(&curPos);
	XMVECTOR xmCenter = XMLoadFloat3(&center);

	XMVECTOR cur = xmCurPos - xmCenter;

	cur = XMVector3Normalize(XMVector3Rotate(cur, quat))*mMoveInfo.radius;

	xmCurPos = xmCenter + cur;

	XMStoreFloat3(&curPos, xmCurPos);

	mCharacter->SetAccumulatedPosition(curPos);
}