/*
 * Copyright 2024, Mibi88 <mbcontact50@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */


#include "App.h"
#include "MainWindow.h"

#include <AboutWindow.h>
#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Application"

const char* kApplicationSignature = "application/x-vnd.Mibi88-HexSpy";


App::App() : BApplication(kApplicationSignature) {
	MainWindow* mainWindow = new MainWindow();
	mainWindow->Show();
}


App::~App() {
}


void App::AboutRequested() {
	BAboutWindow* about
		= new BAboutWindow(B_TRANSLATE_SYSTEM_NAME("HexSpy"), kApplicationSignature);
	about->AddDescription(B_TRANSLATE("A small native hex editor for Haiku."));
	about->AddCopyright(2024, "Mibi88");
	about->Show();
}


int main() {
	App* app = new App();
	app->Run();
	delete app;
	return 0;
}
