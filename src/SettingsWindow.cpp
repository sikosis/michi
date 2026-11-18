#include "SettingsWindow.h"

#include "Constants.h"

#include <Button.h>
#include <ColorControl.h>
#include <LayoutBuilder.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <PopUpMenu.h>
#include <StringView.h>

namespace {
enum : uint32 { kCancel = 'scan' };

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

SettingsWindow::SettingsWindow(const Settings& settings, BMessenger target)
	: BWindow(BRect(0, 0, 430, 360), kSettingsWindowTitle, B_TITLED_WINDOW,
		B_NOT_ZOOMABLE | B_NOT_RESIZABLE | B_AUTO_UPDATE_SIZE_LIMITS),
	  fSettings(settings), fTarget(target) {
	BPopUpMenu* fields = new BPopUpMenu("field");
	fields->SetRadioMode(true);
	BMenuItem* created = new BMenuItem("Created date", nullptr);
	BMenuItem* modified = new BMenuItem("Modified date", nullptr);
	fields->AddItem(created);
	fields->AddItem(modified);
	(settings.sortField == SortField::Created ? created : modified)->SetMarked(true);
	fField = new BMenuField("Sort projects by:", fields);

	BPopUpMenu* directions = new BPopUpMenu("direction");
	directions->SetRadioMode(true);
	BMenuItem* oldest = new BMenuItem("Oldest first", nullptr);
	BMenuItem* newest = new BMenuItem("Newest first", nullptr);
	directions->AddItem(oldest);
	directions->AddItem(newest);
	(settings.ascending ? oldest : newest)->SetMarked(true);
	fDirection = new BMenuField("Order:", directions);

	fColor = new BColorControl(B_ORIGIN, B_CELLS_32x8, 6, "default_color", nullptr);
	fColor->SetValue(Unpack(settings.defaultColor));
	BButton* cancel = new BButton("Cancel", new BMessage(kCancel));
	BButton* save = new BButton("Save", new BMessage(kMsgSaveSettings));
	save->MakeDefault(true);

	BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_DEFAULT_SPACING)
		.SetInsets(B_USE_WINDOW_INSETS)
		.Add(fField)
		.Add(fDirection)
		.Add(new BStringView("default_label", "Default Colour for New Projects:"))
		.Add(fColor)
		.AddGroup(B_HORIZONTAL)
			.AddGlue()
			.Add(cancel)
			.Add(save)
		.End();
	CenterOnScreen();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
SettingsWindow::MessageReceived(BMessage* message) {
	if (message->what == kCancel) {
		Quit();
		return;
	}
	if (message->what == kMsgSaveSettings) {
		BMenuItem* selected = fField->Menu()->FindMarked();
		fSettings.sortField = selected == fField->Menu()->ItemAt(0)
			? SortField::Created : SortField::Modified;
		fSettings.ascending = fDirection->Menu()->FindMarked()
			== fDirection->Menu()->ItemAt(0);
		fSettings.defaultColor = Pack(fColor->ValueAsColor());
		BMessage update(kMsgSettingsChanged);
		update.AddInt32("sort_field", static_cast<int32>(fSettings.sortField));
		update.AddBool("ascending", fSettings.ascending);
		update.AddUInt32("default_color", fSettings.defaultColor);
		fTarget.SendMessage(&update);
		Quit();
		return;
	}
	BWindow::MessageReceived(message);
}
//---------------------------------------------------------------------------------------------------------------------------------//
