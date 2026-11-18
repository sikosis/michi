#ifndef MICHI_PROJECT_EDITOR_H
#define MICHI_PROJECT_EDITOR_H

#include "Project.h"

#include <Messenger.h>
#include <Window.h>

class BColorControl;
class BSlider;
class BTextControl;
class BTextView;

class ProjectEditor : public BWindow {
public:
	ProjectEditor(const Project* project, uint32 defaultColor, BMessenger target);
	void MessageReceived(BMessage* message) override;
	bool QuitRequested() override;

private:
	void _Save();
	void _RandomiseColour();
	Project fProject;
	BMessenger fTarget;
	BTextControl* fName;
	BSlider* fProgress;
	BTextView* fNotes;
	BColorControl* fColor;
};

#endif
