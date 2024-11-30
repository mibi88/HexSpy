/*
 * Copyright 2024, Mibi88 <mbcontact50@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */


#include "MainWindow.h"

#include <Application.h>
#include <Catalog.h>
#include <File.h>
#include <FindDirectory.h>
#include <LayoutBuilder.h>
#include <Menu.h>
#include <MenuBar.h>
#include <Path.h>
#include <View.h>
#include <ScrollView.h>
#include "HexView.h"

#include <cstdio>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Window"

static const uint32 kMsgNewFile = 'fnew';
static const uint32 kMsgOpenFile = 'fopn';
static const uint32 kMsgSaveFile = 'fsav';

static const char* kSettingsFile = "MyApplication_settings";


MainWindow::MainWindow() :
	BWindow(BRect(100, 100, 500, 400), B_TRANSLATE("HexSpy"), B_TITLED_WINDOW,
		B_ASYNCHRONOUS_CONTROLS | B_QUIT_ON_WINDOW_CLOSE) {
	BMenuBar* menuBar = _BuildMenu();
	hexView = new HexView("hexview", 0);
	hexView->SetContent((unsigned char*)"HexSpy\n", 7);
	printf("hexView: %p\n", hexView);
	BScrollView* scrollView = new BScrollView("scrollview", hexView,
			B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE, false, true);

	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.Add(menuBar)
		.Add(scrollView, FLT_MAX)
		.AddGlue()
		.End();

	BMessenger messenger(this);
	fOpenPanel = new BFilePanel(B_OPEN_PANEL, &messenger, NULL, B_FILE_NODE, false);
	fSavePanel = new BFilePanel(B_SAVE_PANEL, &messenger, NULL, B_FILE_NODE, false);

	BMessage settings;
	_LoadSettings(settings);

	BRect frame;
	if (settings.FindRect("main_window_rect", &frame) == B_OK){
		MoveTo(frame.LeftTop());
		ResizeTo(frame.Width(), Bounds().Height());
	}
	MoveOnScreen();
}


MainWindow::~MainWindow() {
	_SaveSettings();

	delete fOpenPanel;
	delete fSavePanel;
}


void MainWindow::MessageReceived(BMessage* message) {
	entry_ref ref;
	const char* name;
	BPath path;
	switch (message->what) {
		case B_SIMPLE_DATA:
		case B_REFS_RECEIVED:
			if(message->FindRef("refs", &ref) == B_OK){
				fSaveMenuItem->SetEnabled(true);
				printf("File opened/dropped\n");
				BEntry entry(&ref, true);
				entry.GetPath(&path);
				printf("Open path: %s\n", path.Path());
				FILE *fp = fopen(path.Path(), "r");
				if(fp){
					hexView->SetContentFromFile(fp);
					fclose(fp);
				}else{
					// TODO: Show an error message.
				}
			}
			break;
		case B_SAVE_REQUESTED:
			if(message->FindRef("directory", &ref) == B_OK &&
				message->FindString("name", &name) == B_OK) {
				BDirectory directory(&ref);
				BEntry entry(&directory, name);
				path = BPath(&entry);

				printf("Save path: %s\n", path.Path());
				
				FILE *fp = fopen(path.Path(), "w");
				fwrite(hexView->Buffer(), 1, hexView->BufferSize(), fp);
				fclose(fp);
			}
			break;
		case kMsgNewFile:
			fSaveMenuItem->SetEnabled(false);
			printf("New\n");
			// TODO: Show a dialog if file not saved.
			hexView->SetContent((unsigned char*)"\x00", 1);
			break;

		case kMsgOpenFile:
			fOpenPanel->Show();
			break;
		case kMsgSaveFile:
			fSavePanel->Show();
			break;
		case B_KEY_DOWN:
			//puts("key down");
			ssize_t size;
			const char *utf8;
			message->FindData("byte", B_INT8_TYPE, (const void**)&utf8, &size);
			//printf("Char: %c, size: %ld\n", utf8[0], size);
			//hexView->KeyDown(utf8, size);
			break;
		default:
			BWindow::MessageReceived(message);
			break;
	}
}


BMenuBar* MainWindow::_BuildMenu() {
	BMenuBar* menuBar = new BMenuBar("menubar");
	BMenu* menu;
	BMenuItem* item;

	// menu 'File'
	menu = new BMenu(B_TRANSLATE("File"));

	item = new BMenuItem(B_TRANSLATE("New"), new BMessage(kMsgNewFile), 'N');
	menu->AddItem(item);

	item = new BMenuItem(B_TRANSLATE("Open" B_UTF8_ELLIPSIS), new BMessage(kMsgOpenFile), 'O');
	menu->AddItem(item);

	fSaveMenuItem = new BMenuItem(B_TRANSLATE("Save"), new BMessage(kMsgSaveFile), 'S');
	fSaveMenuItem->SetEnabled(true);
	menu->AddItem(fSaveMenuItem);

	menu->AddSeparatorItem();

	item = new BMenuItem(B_TRANSLATE("About" B_UTF8_ELLIPSIS), new BMessage(B_ABOUT_REQUESTED));
	item->SetTarget(be_app);
	menu->AddItem(item);

	item = new BMenuItem(B_TRANSLATE("Quit"), new BMessage(B_QUIT_REQUESTED), 'Q');
	menu->AddItem(item);

	menuBar->AddItem(menu);

	return menuBar;
}


status_t MainWindow::_LoadSettings(BMessage& settings) {
	BPath path;
	status_t status;
	status = find_directory(B_USER_SETTINGS_DIRECTORY, &path);
	if (status != B_OK)
		return status;

	status = path.Append(kSettingsFile);
	if (status != B_OK)
		return status;

	BFile file;
	status = file.SetTo(path.Path(), B_READ_ONLY);
	if (status != B_OK)
		return status;

	return settings.Unflatten(&file);
}


status_t MainWindow::_SaveSettings() {
	BPath path;
	status_t status = find_directory(B_USER_SETTINGS_DIRECTORY, &path);
	if (status != B_OK)
		return status;

	status = path.Append(kSettingsFile);
	if (status != B_OK)
		return status;

	BFile file;
	status = file.SetTo(path.Path(), B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
	if (status != B_OK)
		return status;

	BMessage settings;
	status = settings.AddRect("main_window_rect", Frame());

	if (status == B_OK)
		status = settings.Flatten(&file);

	return status;
}
