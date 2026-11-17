#include "DeskbarView.h"

#include "Constants.h"

#include <ClassInfo.h>
#include <Deskbar.h>
#include <InterfaceDefs.h>
#include <MenuItem.h>
#include <Messenger.h>
#include <PopUpMenu.h>
#include <Roster.h>
#include <Window.h>

#include <algorithm>

DeskbarView::DeskbarView(BRect frame)
	: BView(frame, kDeskbarItemName, B_FOLLOW_NONE, B_WILL_DRAW) {
}
//---------------------------------------------------------------------------------------------------------------------------------//


DeskbarView::DeskbarView(BMessage* archive)
	: BView(archive) {
}
//---------------------------------------------------------------------------------------------------------------------------------//


BArchivable*
DeskbarView::Instantiate(BMessage* archive) {
	if (!validate_instantiation(archive, "DeskbarView"))
		return nullptr;
	return new DeskbarView(archive);
}
//---------------------------------------------------------------------------------------------------------------------------------//


status_t
DeskbarView::Archive(BMessage* archive, bool deep) const {
	status_t status = BView::Archive(archive, deep);
	if (status != B_OK)
		return status;
	return archive->AddString("add_on", kAppSignature);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
DeskbarView::AttachedToWindow() {
	BView::AttachedToWindow();
	SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
	SetLowUIColor(B_PANEL_BACKGROUND_COLOR);
	SetToolTip("Michi projects");
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
DeskbarView::Draw(BRect) {
	BRect bounds(Bounds());
	float inset = std::max(2.0f, bounds.Height() * 0.16f);
	BRect icon(bounds.InsetByCopy(inset, inset));
	SetPenSize(std::max(2.0f, bounds.Height() * 0.13f));
	SetLineMode(B_ROUND_CAP, B_ROUND_JOIN);
	SetHighColor(70, 89, 110);
	BPoint a(icon.left, icon.bottom - icon.Height() * 0.15f);
	BPoint b(icon.left + icon.Width() * 0.32f, icon.top + icon.Height() * 0.52f);
	BPoint c(icon.left + icon.Width() * 0.62f, icon.top + icon.Height() * 0.68f);
	BPoint d(icon.right, icon.top + icon.Height() * 0.12f);
	StrokeLine(a, b);
	StrokeLine(b, c);
	SetHighColor(59, 130, 246);
	StrokeLine(c, d);
	SetPenSize(1);
	FillEllipse(d, std::max(2.0f, bounds.Height() * 0.11f),
		std::max(2.0f, bounds.Height() * 0.11f));
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
DeskbarView::MouseDown(BPoint where) {
	int32 buttons = 0;
	if (Window() != nullptr && Window()->CurrentMessage() != nullptr)
		Window()->CurrentMessage()->FindInt32("buttons", &buttons);
	if ((buttons & B_SECONDARY_MOUSE_BUTTON) != 0) {
		_ShowMenu(where);
		return;
	}
	if ((buttons & B_PRIMARY_MOUSE_BUTTON) != 0)
		_Send(kMsgShowMain);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
DeskbarView::_Send(uint32 what) {
	BMessage message(what);
	BMessenger app(kAppSignature);
	if (app.IsValid() && app.SendMessage(&message) == B_OK)
		return;
	be_roster->Launch(kAppSignature, &message);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
DeskbarView::_ShowMenu(BPoint where) {
	BPopUpMenu* menu = new BPopUpMenu("Michi menu", false, false);
	menu->AddItem(new BMenuItem("Settings" B_UTF8_ELLIPSIS,
		new BMessage(kMsgShowSettings)));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem("About Michi" B_UTF8_ELLIPSIS,
		new BMessage(kMsgShowAbout)));
	menu->AddItem(new BMenuItem("Quit", new BMessage(kMsgDeskbarQuit)));
	BMenuItem* selected = menu->Go(ConvertToScreen(where), false, true);
	uint32 what = selected != nullptr && selected->Message() != nullptr
		? selected->Message()->what : 0;
	delete menu;
	if (what == kMsgDeskbarQuit) {
		BDeskbar deskbar;
		deskbar.RemoveItem(kDeskbarItemName);
		BMessenger app(kAppSignature);
		if (app.IsValid())
			app.SendMessage(B_QUIT_REQUESTED);
	} else if (what != 0) {
		_Send(what);
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//


BView*
instantiate_deskbar_item(float maxWidth, float maxHeight) {
	float size = std::min(maxWidth, maxHeight);
	return new DeskbarView(BRect(0, 0, size - 1, size - 1));
}
//---------------------------------------------------------------------------------------------------------------------------------//
