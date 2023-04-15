#pragma once

#include <string>
#include "framework.h"

struct IfError
{
	/*
	* hr이 에러라면 에러를 throw한다.
	*/
	static void									Throw(HRESULT&& hr,std::wstring msg)
	{
		if (hr != S_OK)
			throw IfError{ msg,hr };
	}
	/*
	* msg를 이용해서 throw한다.
	*/
	static void									Throw(std::wstring msg)
	{
		throw IfError{ msg,0 };
	}
	/*
	* 에러 메세지를 나타낸다. 
	*/
	std::wstring								errorMsg;
	/*
	* 에러 코드를 나타낸다.
	*/
	HRESULT										errorCode;
};