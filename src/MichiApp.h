#ifndef MICHI_APP_H
#define MICHI_APP_H

#include "Project.h"

#include <Application.h>

class MainWindow;

class MichiApp : public BApplication {
public:
	MichiApp();
	void ReadyToRun() override;
	void ArgvReceived(int32 argc, char** argv) override;
	void MessageReceived(BMessage* message) override;
	void AboutRequested() override;

private:
	void _InstallDeskbarItem();
	void _ShowMain();
	void _ShowSettings();
	void _SaveProject(BMessage* message);
	void _ApplySettings(BMessage* message);

	ProjectStore fStore;
	SettingsStore fSettingsStore;
	Settings fSettings;
	MainWindow* fMainWindow;
	bool fInitialMessageHandled;
};

#endif
