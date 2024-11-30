/*
 * Copyright 2024, Mibi88 <mbcontact50@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */


#include "HexView.h"
#include <ScrollBar.h>
#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <cstring>

enum {
	KEY_INSERT = 0x05,
	KEY_BACKSPACE = 0x08,
	KEY_LEFT = 0x1C,
	KEY_RIGHT,
	KEY_UP,
	KEY_DOWN
};

HexView::HexView(const char *name, uint32 flags, BLayout *layout) :
		BView(name, flags | B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE, layout) {
	hexGroup = 2;
	bufferSize = 0;
	buffer = NULL;
	editable = true;
	hexPanelSelected = true;
	cur = 0;
	insert = true;
}

void HexView::AttachedToWindow() {
	SetFont(be_fixed_font);
	SetFontSize(12);
	digitWidth = be_fixed_font->StringWidth("a");
	//printf("Digit width %f\n", digitWidth);
	be_fixed_font->GetHeight(&digitHeight);
	MakeFocus(true);
}

void HexView::Draw(BRect rect) {
	int x, y, n;
	const char digits[17] = "0123456789ABCDEF";
	BScrollBar *scrollBar;
	float viewSize = ((float)bufferSize/groupNum+1)*(digitHeight.ascent+digitHeight.leading);
	viewWidth = (int)(Frame().Width()/digitWidth);
	groupNum = viewWidth/(hexGroup+2);
	printf("Digit view size: %d\n", viewWidth);
	printf("Digit view size: %d\n", groupNum);
	//SetLowColor(255, 255, 255);
	//FillRect(*new BRect(0, 0, 10, 10));
	SetHighColor(0, 0, 0);
	charX = Frame().Width()-1-groupNum*digitWidth;
	float hexEnd = groupNum*(hexGroup+1)*digitWidth-digitWidth;
	float diff = charX-hexEnd;
	int start = (int)(rect.top/(digitHeight.ascent+digitHeight.leading));
	int end = (int)(rect.bottom/(digitHeight.ascent+digitHeight.leading))+1;
	printf("Start: %d, end: %d\n", start, end);
	size_t pos = start*groupNum;
	printf("Pos: %ld\n", pos);
	//SetExplicitMinSize(BSize(0, (int)((float)bufferSize/groupNum)+1));
	//SetExplicitPreferredSize(BSize(0, (int)((float)bufferSize/groupNum)+1));
	if((scrollBar = ScrollBar(B_VERTICAL))){
		puts("Scrollbar found");
		scrollBar->SetProportion(scrollBar->Frame().Height()/
				(viewSize-scrollBar->Frame().Height()));
		scrollBar->SetRange(0, (viewSize-scrollBar->Frame().Height()));
	}
	for(y=start;y<=end+1;y++){
		if(pos >= bufferSize) break;
		MovePenTo(*new BPoint(1, (digitHeight.ascent+digitHeight.leading)*(y+1)));
		for(x=0;x<groupNum;x++){
			if(pos+x >= bufferSize) break;
			for(n=0;n<hexGroup;n++){
				if((pos+x)*2+n == cur){
					SetHighColor(0, 0, 0);
					if(hexPanelSelected){
						FillRect(*new BRect(1+(x*3+n)*digitWidth,
								(digitHeight.ascent+digitHeight.leading)*(y),
								1+(x*3+n+1)*digitWidth,
								(digitHeight.ascent+digitHeight.leading)*(y+1)+1));
					}else{
						StrokeRect(*new BRect(1+(x*3+n)*digitWidth,
								(digitHeight.ascent+digitHeight.leading)*(y),
								1+(x*3+n+1)*digitWidth,
								(digitHeight.ascent+digitHeight.leading)*(y+1)+1));
					}
					MovePenTo(*new BPoint(1+(x*3+n)*digitWidth,
							(digitHeight.ascent+digitHeight.leading)*(y+1)));
					if(hexPanelSelected){
						SetHighColor(255, 255, 255);
					}
				}
				DrawString(digits+((buffer[pos+x]>>(!(n&1)*4))&0xF), 1);
				SetHighColor(0, 0, 0);
			}
			DrawString(" ", 1);
		}
		MovePenTo(*new BPoint(charX, (digitHeight.ascent+digitHeight.leading)*(y+1)));
		for(x=0;x<groupNum;x++){
			if(pos+x >= bufferSize) break;
			if(pos+x == cur/2){
				if(!hexPanelSelected){
					FillRect(*new BRect(charX+x*digitWidth,
							(digitHeight.ascent+digitHeight.leading)*y,
							charX+x*digitWidth+digitWidth,
							(digitHeight.ascent+digitHeight.leading)*(y+1)+1));
					SetHighColor(255, 255, 255);
				}else{
					StrokeRect(*new BRect(charX+x*digitWidth,
							(digitHeight.ascent+digitHeight.leading)*y,
							charX+x*digitWidth+digitWidth,
							(digitHeight.ascent+digitHeight.leading)*(y+1)+1));
				}
				MovePenTo(*new BPoint(charX+x*digitWidth,
						(digitHeight.ascent+digitHeight.leading)*(y+1)));
			}
			if(isprint((char)buffer[pos+x])){
				DrawString((char*)buffer+pos+x, 1);
			}
			else DrawString(".", 1);
			SetHighColor(0, 0, 0);
		}
		pos += groupNum;
	}
	if(cur >= bufferSize*2){
		if(x >= groupNum){
			x = 0;
		}else{
			y--;
		}
		SetHighColor(0, 0, 0);
		if(hexPanelSelected){
			FillRect(*new BRect(1+(x*3)*digitWidth,
					(digitHeight.ascent+digitHeight.leading)*(y),
					1+(x*3+1)*digitWidth,
					(digitHeight.ascent+digitHeight.leading)*(y+1)+1));
			StrokeRect(*new BRect(charX+x*digitWidth,
					(digitHeight.ascent+digitHeight.leading)*y,
					charX+x*digitWidth+digitWidth,
					(digitHeight.ascent+digitHeight.leading)*(y+1)+1));
		}else{
			StrokeRect(*new BRect(1+(x*3)*digitWidth,
					(digitHeight.ascent+digitHeight.leading)*(y),
					1+(x*3+1)*digitWidth,
					(digitHeight.ascent+digitHeight.leading)*(y+1)+1));
			FillRect(*new BRect(charX+x*digitWidth,
					(digitHeight.ascent+digitHeight.leading)*y,
					charX+x*digitWidth+digitWidth,
					(digitHeight.ascent+digitHeight.leading)*(y+1)+1));
		}
	}
	StrokeLine(*new BPoint(hexEnd+diff/2, 1), *new BPoint(hexEnd+diff/2,
			MAX(Frame().Height()-1, viewSize-1)));
	//FillRect(*new BRect(0, 0, 10, 10));
}

void HexView::Delete() {
	if(cur < 2) return;
	// Realloc the buffer
	unsigned char *newBuffer = (unsigned char*)malloc(bufferSize-1);
	if(!newBuffer){
		// TODO: Show an error message.
		return;
	}
	memcpy(newBuffer, buffer, (cur>>1)-1);
	if(cur>>1 < bufferSize) memcpy(newBuffer+(cur>>1)-1, buffer+(cur>>1), bufferSize-1-(cur>>1));
	free(buffer);
	buffer = newBuffer;
	bufferSize--;
	cur -= 2;
	Invalidate();
}

void HexView::Edit(char value) {
	const char digits[17] = "0123456789ABCDEF";
	int idx;
	if(insert){
		if(hexPanelSelected && strchr(digits, toupper(value))){
			idx = strchr(digits, toupper(value))-digits;
			if(cur&1){
				/* The last nibble is selected. We should just replace it. */
				buffer[cur>>1] &= 0xF0;
				buffer[cur>>1] |= idx&0xF;
				printf("Last nibble. Idx: %x\n", idx);
				// Move the cursor
				cur++;
			}else{
				/* The first nibble is selected. We should resize the buffer by one, memove it and
				 * set the first nibble. If the user has the cursor at the end of the buffer, we
				 * should insert a byte an set it to the value of the first nibble.
				 * (maybe what I wrote isn't very clear, sorry)
				 */
				// Realloc the buffer
				unsigned char *newBuffer = (unsigned char*)malloc(bufferSize+1);
				if(!newBuffer){
					// TODO: Show an error message.
					return;
				}
				memcpy(newBuffer, buffer, bufferSize);
				free(buffer);
				buffer = newBuffer;
				bufferSize++;
				// Memove the data
				memmove(buffer+(cur>>1)+1, buffer+(cur>>1), bufferSize-(cur>>1)-1);
				// Set the value
				printf("Idx: %x\n", idx);
				buffer[cur>>1] = (idx&0xF)<<4;
				// Move the cursor
				cur++;
			}
		}else if(!hexPanelSelected && isprint(value)){
			/* Insert a byte set to the value at the cursor pos. */
			// Realloc the buffer
			unsigned char *newBuffer = (unsigned char*)malloc(bufferSize+1);
			if(!newBuffer){
				// TODO: Show an error message.
				return;
			}
			memcpy(newBuffer, buffer, bufferSize);
			free(buffer);
			buffer = newBuffer;
			bufferSize++;
			// Memove the data
			memmove(buffer+(cur>>1)+1, buffer+(cur>>1), bufferSize-(cur>>1)-1);
			// Set the value
			buffer[cur>>1] = value;
			// Move the cursor
			cur += 2;
		}
	}else{
		if(hexPanelSelected && strchr(digits, toupper(value))){
			idx = strchr(digits, toupper(value))-digits;
			/* Replace the nibble */
			buffer[cur>>1] &= 0xF<<(4*(cur&1));
			buffer[cur>>1] |= (idx&0xF)<<(4*(!(cur&1)));
			if(cur < bufferSize*2-1) cur++;
		}else if(!hexPanelSelected && isprint(value)){
			/* Replace the character */
			buffer[cur>>1] = value;
			if(cur < bufferSize*2-2) cur += 2;
		}
	}
	Invalidate();
}

void HexView::KeyDown(const char *bytes, int32 numBytes) {
	const char digits[17] = "0123456789ABCDEF";
	float curY, curYMax;
	BScrollBar *scrollBar;
	for(int i=0;i<numBytes;i++){
		printf("Byte from keydown: %x (%c)\n", bytes[i], bytes[i]);
	}
	if(numBytes > 0){
		switch(bytes[0]){
			case KEY_LEFT:
				if(cur > !hexPanelSelected) cur-=1+!hexPanelSelected;
				Invalidate();
				break;
			case KEY_RIGHT:
				if(cur+!hexPanelSelected < (bufferSize*2)-(!insert)) cur+=1+!hexPanelSelected;
				Invalidate();
				break;
			case KEY_DOWN:
				if(cur+groupNum*2 < (bufferSize*2)-(!insert)) cur += groupNum*2;
				Invalidate();
				break;
			case KEY_UP:
				if(cur >= groupNum*2) cur -= groupNum*2;
				Invalidate();
				break;
			case KEY_BACKSPACE:
				if(editable) Delete();
				break;
			case KEY_INSERT:
				if(editable) insert = !insert;
				Invalidate();
				break;
			default:
				if(editable) Edit(bytes[0]);
		}
		if((scrollBar = ScrollBar(B_VERTICAL))){
			curY = ((float)cur/(groupNum*2))*(digitHeight.ascent+digitHeight.leading);
			curYMax = curY + (digitHeight.ascent+digitHeight.leading);
			if(curY < scrollBar->Value()){
				scrollBar->SetValue(curY);
			}else if(curYMax > scrollBar->Value()+Frame().Height()){
				scrollBar->SetValue(curYMax-Frame().Height());
			}
		}
	}
}

void HexView::MouseDown(BPoint point) {
	puts("Mouse down");
	printf("Mouse position: %f, %f\n", point.x, point.y);
	float maxX = (groupNum*(hexGroup+1)-1)*digitWidth;
	printf("Max x: %f\n", maxX);
	if(point.x < maxX){
		hexPanelSelected = true;
		puts("Moving the cursor");
		cur = (int)(point.y/(digitHeight.ascent+digitHeight.leading))*groupNum*2;
		cur += (int)(point.x/digitWidth/3*2);
		printf("Cursor position: %d\n", cur);
		Invalidate();
	}else{
		puts("Click in the ASCII view");
		cur = (int)(point.y/(digitHeight.ascent+digitHeight.leading))*groupNum*2;
		cur += (int)((point.x-charX)/digitWidth)*2;
		hexPanelSelected = false;
		Invalidate();
	}
	if(cur < 0) cur = 0;
	if(cur >= bufferSize*2-(!insert)) cur = bufferSize*2-(!insert);
}

/*void HexView::FrameResized(float *width, float *height) {
	viewWidth = (int)(*width/digitWidth);
	digitViewWidth = viewWidth/hexGroup;
	puts("resize");
}*/

void HexView::SetContent(unsigned char *data, size_t size) {
	unsigned char *newBuffer = (unsigned char*)malloc(size);
	if(!newBuffer){
		// TODO: Show an error message.
		return;
	}
	memcpy(newBuffer, data, size);
	free(buffer);
	buffer = newBuffer;
	bufferSize = size;
	Invalidate();
}

void HexView::SetContentFromFile(FILE *fp) {
	fseek(fp, 0, SEEK_END);
	size_t size = ftell(fp);
	rewind(fp);
	unsigned char *newBuffer = (unsigned char*)malloc(size);
	if(!newBuffer){
		// TODO: Show an error message.
		return;
	}
	fread(newBuffer, size, 1, fp);
	free(buffer);
	buffer = newBuffer;
	bufferSize = size;
	Invalidate();
}

unsigned char *HexView::Buffer() {
	return buffer;
}

size_t HexView::BufferSize() {
	return bufferSize;
}

HexView::~HexView() {
	free(buffer);
}
