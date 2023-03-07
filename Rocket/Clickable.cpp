#include "Clickable.h"
#include "Game.h"

Clickable::Clickable(const char* fileName, const wchar_t* name)
	: Model(fileName,name)
{

}

unique_ptr<Clickable> Clickable::Spawn(const char* fileName, const wchar_t* name,XMFLOAT3 pos)
{
	unique_ptr<Clickable> m = make_unique<Clickable>(fileName,name);
	m->SetPosition(pos);
	m->Load();
	//현재는 sv는 생성이 안되어서 텍스처가 안보인다.

	return move(m);
}
