#pragma once

#include <string>
#include "framework.h"

struct IfError
{
	static void Throw(HRESULT&& hr,std::wstring msg)
	{
		if (hr != S_OK)
			throw IfError{ msg,hr };
	}

	static void Throw(std::wstring msg)
	{
		throw IfError{ msg,0 };
	}

	std::wstring errorMsg;
	HRESULT errorCode;
};