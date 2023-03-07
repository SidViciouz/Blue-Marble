#pragma once

#include "Util.h"
#include <Queue>
#include "Model.h"
#include "Clickable.h"
#include "Unclickable.h"
#include "Volume.h"

struct SpawnData
{
	string										type;
	const char*									fileName;
	const wchar_t*								textureName;
	XMFLOAT3									pos;
};

struct DestroyData
{
	string										type;
	const char*									name;
};

class SpawnSystem
{
public:
	void										Spawn(shared_ptr<Clickables> mModels, shared_ptr<Unclickables> mWorld,shared_ptr<Volumes> mVolume);
	/*
	* destroy 메서드는 이전 프레임의 GPU에서 destroy하려는 해당 자원을 사용하고 있기 때문에, 이를 기다려야한다.
	* 따라서, 비싼 연산이다. 이는 최적화할 수 있는 여지가 있다.
	*/
	void										Destroy(shared_ptr<Clickables> mModels, shared_ptr<Unclickables> mWorld, shared_ptr<Volumes> mVolume);
	void										SpawnPush(SpawnData&& s);
	void										DestroyPush(DestroyData&& d);
	/*
	* destroy Queue가 비어있는지 여부를 반환한다.
	*/
	bool										IsDestroyQueueEmpty() const;

private:
	int											mSpawnNumber = 0;
	queue<SpawnData>							mSpawnQueue;
	queue<DestroyData>							mDestroyQueue;
};
