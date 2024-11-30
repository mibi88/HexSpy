/*
 * Copyright 2024, Mibi88 <mbcontact50@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef HEXVIEW_H
#define HEXVIEW_H


#include <SupportDefs.h>
#include <View.h>
#include <Font.h>
#include <cstdio>

/* TODO:
 * Add support for displaying nibbles, words, longwords, etc. instead of just working with
 * bytes.
 * Use realloc. IDK why I didn't use it.
 */

class HexView : public BView {
public:
	HexView(const char *name, uint32 flags, BLayout *layout = NULL);
	
	virtual void SetContent(unsigned char *data, size_t size);
	virtual void SetContentFromFile(FILE *fp);
	virtual unsigned char *Buffer();
	virtual size_t BufferSize();
	virtual void Edit(char value);
	virtual void Delete();
	
	virtual void Draw(BRect rect);
	/*virtual void FrameResized(float *width, float *height);*/
	virtual void AttachedToWindow();
	virtual void KeyDown(const char *bytes, int32 numBytes);
	virtual void MouseDown(BPoint point);
	
	virtual ~HexView();
private:
	int cursor;
	int cursorX;
	int cursorY;
	
	int groupNum;
	int viewWidth;
	float digitWidth;
	float charX;
	font_height digitHeight;
	
	unsigned char *buffer;
	size_t bufferSize;
	
	int hexGroup;
	int cur;
	
	bool hexPanelSelected;
	
	bool editable;
	bool insert;
};

#endif // HEXVIEW_H
