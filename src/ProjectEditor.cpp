#include "ProjectEditor.h"

#include "Constants.h"

#include <Alert.h>
#include <Button.h>
#include <ColorControl.h>
#include <GroupLayout.h>
#include <LayoutBuilder.h>
#include <ScrollView.h>
#include <Slider.h>
#include <StringView.h>
#include <TextControl.h>
#include <TextView.h>

namespace {
enum : uint32 { kSave = 'esav', kCancel = 'ecan' };

rgb_color Unpack(uint32 value) {
	return {static_cast<uint8>(value >> 24), static_cast<uint8>(value >> 16),
		static_cast<uint8>(value >> 8), static_cast<uint8>(value)};
}
//---------------------------------------------------------------------------------------------------------------------------------//


uint32 Pack(rgb_color color) {
	return (static_cast<uint32>(color.red) << 24) | (static_cast<uint32>(color.green) << 16)
		| (static_cast<uint32>(color.blue) << 8) | color.alpha;
}
//---------------------------------------------------------------------------------------------------------------------------------//

}

ProjectEditor::ProjectEditor(const Project* project, uint32 defaultColor, BMessenger target)
	: BWindow(BRect(0, 0, 500, 470), project == nullptr ? "New project" : "Edit project",
		B_TITLED_WINDOW, B_NOT_ZOOMABLE | B_AUTO_UPDATE_SIZE_LIMITS),
	  fTarget(target) {
	if (project != nullptr)
		fProject = *project;
	else
		fProject.color = defaultColor;

	fName = new BTextControl("Project name:", fProject.name.String(), nullptr);
	fProgress = new BSlider("progress", "Progress", nullptr, 0, 100, B_HORIZONTAL);
	fProgress->SetValue(fProject.progress);
	fProgress->SetHashMarks(B_HASH_MARKS_BOTTOM);
	fProgress->SetHashMarkCount(11);
	fProgress->SetLimitLabels("0%", "100%");
	fNotes = new BTextView("notes");
	fNotes->SetText(fProject.notes.String());
	fNotes->SetWordWrap(true);
	fColor = new BColorControl(B_ORIGIN, B_CELLS_32x8, 6, "color", nullptr);
	fColor->SetValue(Unpack(fProject.color));

	BButton* cancel = new BButton("Cancel", new BMessage(kCancel));
	BButton* save = new BButton("Save", new BMessage(kSave));
	save->MakeDefault(true);

	BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_DEFAULT_SPACING)
		.SetInsets(B_USE_WINDOW_INSETS)
		.Add(fName)
		.Add(fProgress)
		.AddGroup(B_VERTICAL, B_USE_SMALL_SPACING)
			.Add(new BStringView("notes_label", "Notes:"))
			.Add(new BScrollView("notes_scroll", fNotes, 0, false, true), 1)
		.End()
		.Add(new BStringView("color_label", "Progress colour:"))
		.Add(fColor)
		.AddGroup(B_HORIZONTAL)
			.AddGlue()
			.Add(cancel)
			.Add(save)
		.End();
	CenterOnScreen();
	fName->MakeFocus(true);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
ProjectEditor::MessageReceived(BMessage* message) {
	switch (message->what) {
		case kSave: _Save(); break;
		case kCancel: Quit(); break;
		default: BWindow::MessageReceived(message); break;
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
ProjectEditor::_Save() {
	BString name(fName->Text());
	name.Trim();
	if (name.IsEmpty()) {
		(new BAlert("Michi", "Please enter a project name.", "OK"))->Go();
		fName->MakeFocus(true);
		return;
	}
	fProject.name = name;
	fProject.progress = fProgress->Value();
	fProject.notes = fNotes->Text();
	fProject.color = Pack(fColor->ValueAsColor());

	BMessage save(kMsgSaveProject);
	save.AddString("id", fProject.id);
	save.AddString("name", fProject.name);
	save.AddString("notes", fProject.notes);
	save.AddInt32("progress", fProject.progress);
	save.AddUInt32("color", fProject.color);
	save.AddInt64("created", fProject.created);
	fTarget.SendMessage(&save);
	Quit();
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool
ProjectEditor::QuitRequested() {
	return true;
}
//---------------------------------------------------------------------------------------------------------------------------------//
