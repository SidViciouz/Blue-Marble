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

	std::wstring errorMsg;
	HRESULT errorCode;
};