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
	* destroy �޼���� ���� �������� GPU���� destroy�Ϸ��� �ش� �ڿ��� ����ϰ� �ֱ� ������, �̸� ��ٷ����Ѵ�.
	* ����, ��� �����̴�. �̴� ����ȭ�� �� �ִ� ������ �ִ�.
	*/
	void										Destroy(shared_ptr<Clickables> mModels, shared_ptr<Unclickables> mWorld, shared_ptr<Volumes> mVolume);
	void										SpawnPush(SpawnData&& s);
	void										DestroyPush(DestroyData&& d);
	/*
	* destroy Queue�� ����ִ��� ���θ� ��ȯ�Ѵ�.
	*/
	bool										IsDestroyQueueEmpty() const;

private:
	int											mSpawnNumber = 0;
	queue<SpawnData>							mSpawnQueue;
	queue<DestroyData>							mDestroyQueue;
};
