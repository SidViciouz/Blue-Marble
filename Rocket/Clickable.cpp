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
	//����� sv�� ������ �ȵǾ �ؽ�ó�� �Ⱥ��δ�.

	return move(m);
}
