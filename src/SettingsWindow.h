#ifndef MICHI_SETTINGS_WINDOW_H
#define MICHI_SETTINGS_WINDOW_H

#include "Project.h"

#include <Messenger.h>
#include <Window.h>

class BColorControl;
class BMenuField;

class SettingsWindow : public BWindow {
public:
	SettingsWindow(const Settings& settings, BMessenger target);
	void MessageReceived(BMessage* message) override;
	bool QuitRequested() override;
	void SetSettings(const Settings& settings);

private:
	Settings fSettings;
	BMessenger fTarget;
	BMenuField* fField;
	BMenuField* fDirection;
	BColorControl* fColor;
};

#endif
