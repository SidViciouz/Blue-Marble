#pragma once

#include "SceneNode.h"
#include <ft2build.h>
#include FT_FREETYPE_H

class TextNode : public SceneNode
{
public:
												TextNode();
	/*
	* 화면 상에 텍스트를 정해진 위치에 draw한다.
	*/
	virtual void								Draw() override;
	virtual void								Update() override;
	/*
	* 그리고자하는 text를 지정한다.
	*/
	void										SetText(const string& text);
	/*
	* 그리고자하는 text를 반환한다.
	*/
	const string&								GetText() const;

protected:
	/*
	* 그리려고하는 text를 저장한다.
	*/
	string										mText;
	/*
	* 그리려고하는 텍스트의 각 character를 32bit로 변화하여 저장하고 있는 배열이다.
	*/
	int											mText32bits[1000];
	/*
	* 텍스트를 upload할 때 사용되는 upload buffer의 index(handle)이다.
	*/
	int											mUploadBufferIdx;
	/*
	* 텍스트를 upload할 때 사용되는 upload buffer의 srv에 대한 index(handle)이다.
	*/
	int											mUploadBufferSrvIdx;
};
