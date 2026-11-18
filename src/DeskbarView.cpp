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
	BRect tile(bounds.InsetByCopy(1.0f, 1.0f));
	float radius = std::max(3.0f, tile.Height() * 0.22f);
	SetHighColor(31, 98, 190);
	FillRoundRect(tile, radius, radius);
	SetHighColor(112, 181, 255);
	StrokeRoundRect(tile, radius, radius);

	float routeInset = std::max(3.5f, bounds.Height() * 0.22f);
	BRect route(bounds.InsetByCopy(routeInset, routeInset));
	SetPenSize(std::max(2.2f, bounds.Height() * 0.14f));
	SetLineMode(B_ROUND_CAP, B_ROUND_JOIN);
	SetHighColor(255, 255, 255);
	BPoint a(route.left, route.bottom);
	BPoint b(route.left + route.Width() * 0.34f, route.top + route.Height() * 0.52f);
	BPoint c(route.left + route.Width() * 0.64f, route.top + route.Height() * 0.68f);
	BPoint d(route.right, route.top);
	StrokeLine(a, b);
	StrokeLine(b, c);
	StrokeLine(c, d);
	SetHighColor(255, 204, 64);
	float markerRadius = std::max(2.0f, bounds.Height() * 0.12f);
	FillEllipse(d, markerRadius, markerRadius);
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
