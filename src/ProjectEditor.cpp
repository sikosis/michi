#include "ProjectEditor.h"

#include "Constants.h"

#include <Alert.h>
#include <Button.h>
#include <ColorControl.h>
#include <Font.h>
#include <GroupLayout.h>
#include <LayoutBuilder.h>
#include <OS.h>
#include <ScrollView.h>
#include <Slider.h>
#include <Size.h>
#include <StringView.h>
#include <TextControl.h>
#include <TextView.h>

#include <algorithm>

namespace {
enum : uint32 {
	kSave = 'esav',
	kCancel = 'ecan',
	kRandomColour = 'rcol',
	kSetProgress = 'prst',
	kAdjustProgress = 'prad'
};

constexpr uint32 kProjectColours[] = {
	0x3b82f6ff, 0x8b5cf6ff, 0xec4899ff, 0xef4444ff,
	0xf97316ff, 0xeab308ff, 0x22c55eff, 0x06b6d4ff
};

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


BButton*
ProgressButton(const char* label, uint32 what, int32 value) {
	BMessage* message = new BMessage(what);
	message->AddInt32("value", value);
	BButton* button = new BButton(label, message);
	BFont font(*be_plain_font);
	font.SetSize(std::max(9.0f, be_plain_font->Size() - 2.0f));
	button->SetFont(&font);
	float width = what == kSetProgress ? 48.0f : 40.0f;
	button->SetExplicitMinSize(BSize(width, 22));
	button->SetExplicitMaxSize(BSize(width, 22));
	return button;
}
//---------------------------------------------------------------------------------------------------------------------------------//

}

ProjectEditor::ProjectEditor(const Project* project, uint32 defaultColor, BMessenger target)
	: BWindow(BRect(0, 0, 460, 490),
		project == nullptr ? kNewProjectWindowTitle : kEditProjectWindowTitle,
		B_TITLED_WINDOW, B_NOT_ZOOMABLE | B_AUTO_UPDATE_SIZE_LIMITS),
	  fTarget(target) {
	if (project != nullptr)
		fProject = *project;
	else
		fProject.color = defaultColor;

	fName = new BTextControl("Project Name:", fProject.name.String(), nullptr);
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
	BButton* randomColour = new BButton("Random Colour", new BMessage(kRandomColour));
	save->MakeDefault(true);

	BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_DEFAULT_SPACING)
		.SetInsets(B_USE_WINDOW_INSETS)
		.Add(fName)
		.Add(fProgress)
		.AddGroup(B_HORIZONTAL, B_USE_SMALL_SPACING)
			.AddGlue()
			.Add(ProgressButton("0%", kSetProgress, 0))
			.Add(ProgressButton("25%", kSetProgress, 25))
			.Add(ProgressButton("50%", kSetProgress, 50))
			.Add(ProgressButton("75%", kSetProgress, 75))
			.Add(ProgressButton("100%", kSetProgress, 100))
			.AddGlue()
		.End()
		.AddGroup(B_HORIZONTAL, B_USE_SMALL_SPACING)
			.AddGlue()
			.Add(ProgressButton("-20", kAdjustProgress, -20))
			.Add(ProgressButton("-10", kAdjustProgress, -10))
			.Add(ProgressButton("-5", kAdjustProgress, -5))
			.Add(ProgressButton("+5", kAdjustProgress, 5))
			.Add(ProgressButton("+10", kAdjustProgress, 10))
			.Add(ProgressButton("+20", kAdjustProgress, 20))
			.AddGlue()
		.End()
		.AddGroup(B_VERTICAL, B_USE_SMALL_SPACING)
			.Add(new BStringView("notes_label", "Notes:"))
			.Add(new BScrollView("notes_scroll", fNotes, 0, false, true), 1)
		.End()
		.AddGroup(B_HORIZONTAL)
			.Add(new BStringView("color_label", "Progress Colour:"))
			.AddGlue()
			.Add(randomColour)
		.End()
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
		case kRandomColour: _RandomiseColour(); break;
		case kSetProgress: {
			int32 value;
			if (message->FindInt32("value", &value) == B_OK)
				fProgress->SetValue(value);
			break;
		}
		case kAdjustProgress: {
			int32 value;
			if (message->FindInt32("value", &value) == B_OK)
				fProgress->SetValue(std::clamp(fProgress->Value() + value, 0, 100));
			break;
		}
		default: BWindow::MessageReceived(message); break;
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
ProjectEditor::_RandomiseColour() {
	size_t count = sizeof(kProjectColours) / sizeof(kProjectColours[0]);
	size_t index = static_cast<size_t>(system_time()) % count;
	if (kProjectColours[index] == Pack(fColor->ValueAsColor()))
		index = (index + 1) % count;
	fColor->SetValue(Unpack(kProjectColours[index]));
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
ProjectEditor::_Save() {
	BString name(fName->Text());
	name.Trim();
	if (name.IsEmpty()) {
		(new BAlert("Michi", "Please enter a Project Name.", "OK"))->Go();
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
