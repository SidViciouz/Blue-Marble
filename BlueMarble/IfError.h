#pragma once

#include <string>
#include "framework.h"

struct IfError
{
	/*
	* hr�� ������� ������ throw�Ѵ�.
	*/
	static void									Throw(HRESULT&& hr,std::wstring msg)
	{
		if (hr != S_OK)
			throw IfError{ msg,hr };
	}
	/*
	* msg�� �̿��ؼ� throw�Ѵ�.
	*/
	static void									Throw(std::wstring msg)
	{
		throw IfError{ msg,0 };
	}
	/*
	* ���� �޼����� ��Ÿ����. 
	*/
	std::wstring								errorMsg;
	/*
	* ���� �ڵ带 ��Ÿ����.
	*/
	HRESULT										errorCode;
};