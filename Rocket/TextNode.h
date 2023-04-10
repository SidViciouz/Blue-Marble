#pragma once

#include "SceneNode.h"
#include <ft2build.h>
#include FT_FREETYPE_H

class TextNode : public SceneNode
{
public:
												TextNode();
	virtual void								Draw() override;
	virtual void								Update() override;
	void										SetText(const string& text);
	const string&								GetText() const;

protected:
	string										mText;
	int											mText32bits[1000];
	int											mUploadBufferIdx;
	int											mUploadBufferSrvIdx;
};
