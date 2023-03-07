#include "SpawnSystem.h"

void SpawnSystem::Spawn(shared_ptr<Clickables> mModels, shared_ptr<Unclickables> mWorld, shared_ptr<Volumes> mVolume)
{
	while (!mSpawnQueue.empty())
	{
		auto front = mSpawnQueue.front();
		mSpawnQueue.pop();

		if (front.type.compare("Clickable") == 0)
		{
			mModels->insert({ to_string(mSpawnNumber++), Clickable::Spawn(front.fileName,front.textureName,front.pos)});
		}
		else if (front.type.compare("Unclickable") == 0)
		{
			
		}
		else if (front.type.compare("Volume") == 0)
		{

		}

	}
}

void SpawnSystem::Destroy(shared_ptr<Clickables> mModels, shared_ptr<Unclickables> mWorld, shared_ptr<Volumes> mVolume)
{
	while (!mDestroyQueue.empty())
	{
		auto front = mDestroyQueue.front();
		mDestroyQueue.pop();

		if (front.type.compare("Clickable") == 0)
		{
			mModels->erase(front.name);
		}
		else if (front.type.compare("Unclickable") == 0)
		{

		}
		else if (front.type.compare("Volume") == 0)
		{

		}
	}
}

void SpawnSystem::SpawnPush(SpawnData&& s)
{
	mSpawnQueue.push(move(s));
}

void SpawnSystem::DestroyPush(DestroyData&& d)
{
	mDestroyQueue.push(move(d));
}

bool SpawnSystem::IsDestroyQueueEmpty() const
{
	return mDestroyQueue.empty();
}
