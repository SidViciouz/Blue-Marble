#include "WorldNode.h"
#include "Engine.h"
#include "json/json.h"
#include <fstream>
#include <iostream>

WorldNode::WorldNode(string name)
	: MeshNode(name)
{
	mCharacter = make_shared<AnimatedNode>();
	mCharacter->SetRelativePosition(0, 0, 30);
	mCharacter->SetScale(0.02f, 0.02f, 0.02f);
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
	
	mColorCountryTextureIdx = Engine::mResourceManager->CreateTexture2D(3600, 1800,
		DXGI_FORMAT_R16_SINT, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

	mColorCountryTextureUavIdx = Engine::mDescriptorManager->CreateUav(
		Engine::mResourceManager->GetResource(mColorCountryTextureIdx),
		DXGI_FORMAT_R16_SINT,
		D3D12_UAV_DIMENSION_TEXTURE2D
	);

	Json::Value root;
	std::ifstream config_doc("../Data/world-administrative-boundaries.json", std::ifstream::binary);
	config_doc >> root;
	string str;

	for (int i = 0; i < root.size(); ++i)
	{
		Json::Value field = root[i]["geo_shape"]["geometry"]["coordinates"];

		Country info;

		P minBound = { 90,180 };
		P maxBound = { -90,-180 };

		info.geo.x = root[i]["geo_point_2d"]["lon"].asDouble();
		info.geo.y = root[i]["geo_point_2d"]["lat"].asDouble();

		for (auto it = field.begin(); it != field.end(); it++)
		{
			vector<P> points;
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
						if (coord[1] < minBound.x)
							minBound.x = coord[1];
						if (coord[1] > maxBound.x)
							maxBound.x = coord[1];
						if(coord[0] < minBound.y)
							minBound.y = coord[0];
						if (coord[0] > maxBound.y)
							maxBound.y = coord[0];

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
						if (coord[1] < minBound.x)
							minBound.x = coord[1];
						if (coord[1] > maxBound.x)
							maxBound.x = coord[1];
						if (coord[0] < minBound.y)
							minBound.y = coord[0];
						if (coord[0] > maxBound.y)
							maxBound.y = coord[0];

						points.push_back({ coord[1] ,coord[0] });
						data[1800-(((int)(coord[1]*10)+900))][((int)(coord[0]*10)+1800)] = 1;
					}
				}
			}
			if (points.size() != 0)
			{
				info.points.push_back(points);
			}
		}
		info.minBound = minBound;
		info.maxBound = maxBound;
		info.index = i;
		mCountrys[root[i]["name"].asString()] = info;
	}

	int i = 0;
	for (auto country : mCountrys)
	{
		int area = 0;
		int num = 0;
		for (auto points : country.second.points)
		{
			mCountryInfos[i].countryIndex = country.second.index;
			mCountryInfos[i].areaIndex = area;
			mCountryInfos[i].maxBound = country.second.maxBound;
			mCountryInfos[i].minBound = country.second.minBound;
			int pointSize = points.size();
			if (pointSize > MAX_NUM_POINT)
				mCountryInfos[i].numOfPoint = MAX_NUM_POINT;
			else
				mCountryInfos[i].numOfPoint = pointSize;

			for (int k = 0,p = 0; k < pointSize; ++k)
			{
				if (k- MAX_NUM_POINT * p == MAX_NUM_POINT)
				{
					++i;
					++p;
					mCountryInfos[i].countryIndex = country.second.index;
					mCountryInfos[i].areaIndex = area;
					mCountryInfos[i].maxBound = country.second.maxBound;
					mCountryInfos[i].minBound = country.second.minBound;
					if (pointSize - MAX_NUM_POINT * p > MAX_NUM_POINT)
						mCountryInfos[i].numOfPoint = MAX_NUM_POINT;
					else
						mCountryInfos[i].numOfPoint = pointSize - MAX_NUM_POINT * p;
				}
				
				mCountryInfos[i].points[k-MAX_NUM_POINT*p] = points[k];
			}
			++i;
			++area;
			num += points.size();
		}
	}


	Engine::mResourceManager->UploadTexture2D(mUploadBufferIdx, data, 3600, 1800, 0, 0);
	Engine::mResourceManager->CopyUploadToTexture(mUploadBufferIdx, mBorderTextureIdx, 3600, 1800, 1, DXGI_FORMAT_R8_UNORM, 1);

	mCountryInfoUploadBufferIdx = Engine::mResourceManager->CreateUploadBuffer(i * sizeof(CountryInfo));
	mCountryInfoBufferIdx = Engine::mResourceManager->CreateDefaultBuffer(i * sizeof(CountryInfo));
	mCountryInfoBufferSrvIdx = Engine::mDescriptorManager->CreateSrv(
		Engine::mResourceManager->GetResource(mCountryInfoBufferIdx),
		DXGI_FORMAT_UNKNOWN,
		D3D12_SRV_DIMENSION_BUFFER,
		1,
		i,
		sizeof(CountryInfo)
	);

	Engine::mResourceManager->Upload(mCountryInfoUploadBufferIdx, mCountryInfos, i * sizeof(CountryInfo), 0);
	Engine::mResourceManager->CopyUploadToBuffer(mCountryInfoUploadBufferIdx, mCountryInfoBufferIdx);

	//Draw
	Engine::mCommandList->SetDescriptorHeaps(1, Engine::mDescriptorManager->GetHeapAddress(DescType::UAV));

	Engine::mCommandList->SetComputeRootSignature(Engine::mRootSignatures["ColorCountry"].Get());
	Engine::mCommandList->SetPipelineState(Engine::mPSOs["ColorCountry"].Get());

	Engine::mCommandList->SetComputeRootDescriptorTable(0,
		Engine::mDescriptorManager->GetGpuHandle(mCountryInfoBufferSrvIdx, DescType::SRV));

	Engine::mCommandList->SetComputeRootDescriptorTable(1,
		Engine::mDescriptorManager->GetGpuHandle(mColorCountryTextureUavIdx, DescType::UAV));

	Engine::mCommandList->SetComputeRoot32BitConstant(2, i, 0);

	Engine::mCommandList->Dispatch(113,57,1);
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
	Engine::mCommandList->SetGraphicsRoot32BitConstant(4, mLatestClicked, 1);

	Engine::mCommandList->SetGraphicsRootDescriptorTable(2, Engine::mDescriptorManager->GetGpuHandle(
		Engine::mAllScenes[Engine::mCurrentSceneName]->mShadowMap->GetTextureSrvIdx(), DescType::SRV));

	Engine::mCommandList->SetGraphicsRootDescriptorTable(3, Engine::mDescriptorManager->GetGpuHandle(
		Engine::mTextureManager->GetTextureIndex(mTextureName), DescType::SRV));

	Engine::mCommandList->SetGraphicsRootDescriptorTable(5, Engine::mDescriptorManager->GetGpuHandle(
		Engine::mTextureManager->GetTextureIndex("earth_displacement"), DescType::SRV));

	Engine::mCommandList->SetGraphicsRootDescriptorTable(6, Engine::mDescriptorManager->GetGpuHandle(
		mBorderTextureUavIdx, DescType::SRV));

	Engine::mCommandList->SetGraphicsRootDescriptorTable(7, Engine::mDescriptorManager->GetGpuHandle(
		mColorCountryTextureUavIdx, DescType::SRV));

	Engine::mCommandList->SetGraphicsRootDescriptorTable(8, Engine::mDescriptorManager->GetGpuHandle(
		Engine::mTextureManager->GetTextureIndex("water"), DescType::SRV));

	Engine::mCommandList->SetGraphicsRootDescriptorTable(9, Engine::mDescriptorManager->GetGpuHandle(
		Engine::mTextureManager->GetTextureIndex("water2"), DescType::SRV));

	Engine::mCommandList->SetGraphicsRootDescriptorTable(10, Engine::mDescriptorManager->GetGpuHandle(
		Engine::mTextureManager->GetTextureIndex("earth_normal"), DescType::SRV));

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

	XMFLOAT3 curPos = mCharacter->GetAccumulatedPosition().v;


	XMFLOAT3 center = mAccumulatedPosition.v;
	Vector3 v1(curPos.x - center.x, curPos.y - center.y, curPos.z - center.z);
	Vector3 v2(pos.x - center.x, pos.y - center.y, pos.z - center.z);

	v1.Normalize();
	v2.Normalize();

	mMoveInfo.radius = GetScale().x;
	mMoveInfo.axis = (v1 ^ v2).Normalized().v;
	mMoveInfo.angle = acos(Vector3::Dot(v1, v2));
	mMoveInfo.totalDistance = mMoveInfo.angle * mMoveInfo.radius;
	mMoveInfo.currentDistance = 0;

	mCharacter->PlayStart(2);
}

void WorldNode::PickCountry(const XMFLOAT3& pos)
{
	XMFLOAT4 quat = mAccumulatedQuaternion.Get();
	XMVECTOR xmPos = XMLoadFloat3(&pos);
	XMVECTOR xmQuat = XMLoadFloat4(&quat);
	xmQuat = XMQuaternionInverse(xmQuat);

	//world coordinate -> spherical coordinate
	XMVECTOR xmCenter = XMLoadFloat3(&mAccumulatedPosition.v);
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
	if (longitude > 180)
	{
		longitude -= 360;
	}

	//geographics coordinate -> uv coordinate
	float u = (longitude + 180) / 360.0f;
	float v = (latitude + 90) / 180.0f;

	//printf("latitude : %f, longitude : %f\n", latitude, longitude);
	//printf("u : %f, v : %f\n", u, v);
	for (auto country : mCountrys)
	{
		if(longitude >= country.second.minBound.y &&
			longitude <= country.second.maxBound.y &&
			latitude >= country.second.minBound.x &&
			latitude <= country.second.maxBound.x
			)
		
		{
			int cnt = 0;
			for (auto points : country.second.points)
			{
				for (int i = 0; i < points.size(); ++i)
				{
					float xs = points[i].x;
					float xl;
					float ys = points[i].y;
					float yl;

					if (i == points.size() - 1)
					{
						xl = points[0].x;
						yl = points[0].y;
					}
					else
					{
						xl = points[i + 1].x;
						yl = points[i + 1].y;
					}

					if (xs > xl)
						swap(xs, xl);

					if (ys > yl)
						swap(ys, yl);

					// 다음의 조건을 만족하면서 영역 밖에 있는 경우를 조건에 추가해야함.
					if (longitude >= ys && longitude <= yl && latitude <= xl)
					{
						++cnt;
					}

				}
			}
			if (cnt % 2 == 1)
			{
				mLatestClicked = country.second.index;
				cout << country.first << ":" << country.second.index << "\n";
				break;
			}
		}
	}
}

void WorldNode::UpdateCharacter()
{
	if (!isMoving)
		return;

	float lDeltaTime = Engine::mTimer.GetDeltaTime();
	float lAngularSpeed = 10.0f;
	float lAngularDistance = lAngularSpeed * lDeltaTime;
	mMoveInfo.currentDistance += lAngularDistance;
	
	if (mMoveInfo.currentDistance >= mMoveInfo.totalDistance)
	{
		isMoving = false;
		mCharacter->PlayStart(1);
		return;
	}

	float lAngle = lAngularDistance / mMoveInfo.radius;

	XMVECTOR quat = XMQuaternionRotationAxis(XMLoadFloat3(&mMoveInfo.axis), lAngle);
	//XMVECTOR quat = XMQuaternionRotationAxis(XMLoadFloat3(&mMoveInfo.axis), mMoveInfo.angle);

	XMFLOAT3 curPos = mCharacter->GetAccumulatedPosition().v;
	XMFLOAT3 center = mAccumulatedPosition.v;

	XMVECTOR xmCurPos = XMLoadFloat3(&curPos);
	XMVECTOR xmCenter = XMLoadFloat3(&center);

	XMVECTOR cur = xmCurPos - xmCenter;

	cur = XMVector3Normalize(XMVector3Rotate(cur, quat))*mMoveInfo.radius;

	xmCurPos = xmCenter + cur;

	XMStoreFloat3(&curPos, xmCurPos);

	mCharacter->SetAccumulatedPosition(curPos);

	//

	XMVECTOR up = XMVectorSet(0, 1, 0, 1);
	XMVECTOR axis = XMVector3Cross(up, cur);
	float angle = XMVectorGetX(XMVector3AngleBetweenVectors(up, cur));
	XMFLOAT4 newQuat;
	XMStoreFloat4(&newQuat, XMQuaternionRotationAxis(axis, angle));

	mCharacter->SetAccumulatedQuaternion(newQuat);
}

bool WorldNode::GetIsMoving() const
{
	return isMoving;
}