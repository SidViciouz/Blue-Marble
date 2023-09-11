#pragma once

#include "SceneNode.h"
#include <ft2build.h>
#include FT_FREETYPE_H

class TextNode : public SceneNode
{
public:
												TextNode();
	/*
	* ȭ�� �� �ؽ�Ʈ�� ������ ��ġ�� draw�Ѵ�.
	*/
	virtual void								Draw() override;
	virtual void								Update() override;
	/*
	* �׸������ϴ� text�� �����Ѵ�.
	*/
	void										SetText(const string& text);
	/*
	* �׸������ϴ� text�� ��ȯ�Ѵ�.
	*/
	const string&								GetText() const;

protected:
	/*
	* �׸������ϴ� text�� �����Ѵ�.
	*/
	string										mText;
	/*
	* �׸������ϴ� �ؽ�Ʈ�� �� character�� 32bit�� ��ȭ�Ͽ� �����ϰ� �ִ� �迭�̴�.
	*/
	int											mText32bits[1000];
	/*
	* �ؽ�Ʈ�� upload�� �� ���Ǵ� upload buffer�� index(handle)�̴�.
	*/
	int											mUploadBufferIdx;
	/*
	* �ؽ�Ʈ�� upload�� �� ���Ǵ� upload buffer�� srv�� ���� index(handle)�̴�.
	*/
	int											mUploadBufferSrvIdx;
};
