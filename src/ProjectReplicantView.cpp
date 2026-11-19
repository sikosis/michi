#include "ProjectReplicantView.h"

#include "Constants.h"

#include <ClassInfo.h>
#include <Dragger.h>
#include <Entry.h>
#include <Font.h>
#include <InterfaceDefs.h>
#include <Messenger.h>
#include <NodeMonitor.h>
#include <Roster.h>
#include <String.h>

#include <algorithm>

namespace {
constexpr float kDraggerSize = 8;
constexpr float kInset = 12;
constexpr float kHeaderHeight = 34;
constexpr float kRowHeight = 42;

rgb_color
Unpack(uint32 value) {
	return {static_cast<uint8>(value >> 24), static_cast<uint8>(value >> 16),
		static_cast<uint8>(value >> 8), static_cast<uint8>(value)};
}
//---------------------------------------------------------------------------------------------------------------------------------//
}

ProjectReplicantView::ProjectReplicantView(BRect frame)
	: BView(frame, "michi project display", B_FOLLOW_NONE,
		B_WILL_DRAW | B_FRAME_EVENTS), fWatching(false) {
	_AddDragger();
}
//---------------------------------------------------------------------------------------------------------------------------------//


ProjectReplicantView::ProjectReplicantView(BMessage* archive)
	: BView(archive), fWatching(false) {
}
//---------------------------------------------------------------------------------------------------------------------------------//


ProjectReplicantView::~ProjectReplicantView() {
	_StopWatching();
}
//---------------------------------------------------------------------------------------------------------------------------------//


BArchivable*
ProjectReplicantView::Instantiate(BMessage* archive) {
	if (!validate_instantiation(archive, "ProjectReplicantView"))
		return nullptr;
	return new ProjectReplicantView(archive);
}
//---------------------------------------------------------------------------------------------------------------------------------//


status_t
ProjectReplicantView::Archive(BMessage* archive, bool deep) const {
	status_t status = BView::Archive(archive, deep);
	if (status != B_OK)
		return status;
	return archive->AddString("add_on", kAppSignature);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
ProjectReplicantView::AttachedToWindow() {
	BView::AttachedToWindow();
	SetViewColor(B_TRANSPARENT_COLOR);
	SetLowColor(B_TRANSPARENT_COLOR);
	_StartWatching();
	_Reload();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
ProjectReplicantView::Draw(BRect updateRect) {
	(void)updateRect;
	BRect bounds(Bounds());
	SetDrawingMode(B_OP_ALPHA);
	SetBlendingMode(B_PIXEL_ALPHA, B_ALPHA_OVERLAY);
	BRect panel(bounds.InsetByCopy(1, 1));
	rgb_color background = ui_color(B_PANEL_BACKGROUND_COLOR);
	background.alpha = 215;
	SetHighColor(background);
	FillRoundRect(panel, 8, 8);
	rgb_color border = tint_color(ui_color(B_PANEL_BACKGROUND_COLOR), B_DARKEN_2_TINT);
	border.alpha = 230;
	SetHighColor(border);
	StrokeRoundRect(panel, 8, 8);

	SetFont(be_bold_font);
	SetHighColor(ui_color(B_PANEL_TEXT_COLOR));
	DrawString("projects", BPoint(kInset, kInset + be_bold_font->Size()));
	SetFont(be_plain_font);

	float y = kHeaderHeight;
	int32 visibleCount = std::max<int32>(0,
		static_cast<int32>((bounds.bottom - y - 24) / kRowHeight));
	int32 count = std::min<int32>(visibleCount, static_cast<int32>(fProjects.size()));
	for (int32 index = 0; index < count; ++index) {
		const Project& project = fProjects[static_cast<size_t>(index)];
		SetHighColor(ui_color(B_PANEL_TEXT_COLOR));
		DrawString(project.name.String(), BPoint(kInset, y + 14));

		BString percent;
		percent.SetToFormat("%" B_PRId32 "%%", project.progress);
		float percentWidth = StringWidth(percent.String());
		DrawString(percent.String(), BPoint(bounds.right - kInset - percentWidth, y + 14));

		BRect bar(kInset, y + 23, bounds.right - kInset, y + 29);
		rgb_color barBackground
			= tint_color(ui_color(B_PANEL_BACKGROUND_COLOR), B_DARKEN_2_TINT);
		barBackground.alpha = 160;
		SetHighColor(barBackground);
		FillRoundRect(bar, 3, 3);
		if (project.progress > 0) {
			bar.right = bar.left + bar.Width() * project.progress / 100.0f;
			SetHighColor(Unpack(project.color));
			FillRoundRect(bar, 3, 3);
		}
		y += kRowHeight;
	}

	if (fProjects.empty()) {
		SetHighColor(tint_color(ui_color(B_PANEL_TEXT_COLOR), B_DISABLED_LABEL_TINT));
		DrawString("No projects yet.", BPoint(kInset, y + 14));
	} else if (count < static_cast<int32>(fProjects.size())) {
		BString more;
		more.SetToFormat("+%" B_PRId32 " more", static_cast<int32>(fProjects.size()) - count);
		SetHighColor(tint_color(ui_color(B_PANEL_TEXT_COLOR), B_DISABLED_LABEL_TINT));
		DrawString(more.String(), BPoint(kInset, bounds.bottom - 10));
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
ProjectReplicantView::MessageReceived(BMessage* message) {
	if (message->what == B_NODE_MONITOR) {
		_Reload();
		return;
	}
	BView::MessageReceived(message);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
ProjectReplicantView::MouseDown(BPoint where) {
	(void)where;
	BMessage message(kMsgShowMain);
	BMessenger app(kAppSignature);
	if (app.IsValid() && app.SendMessage(&message) == B_OK)
		return;
	be_roster->Launch(kAppSignature, &message);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
ProjectReplicantView::_AddDragger() {
	BRect frame(Bounds());
	frame.left = frame.right - kDraggerSize;
	frame.top = frame.bottom - kDraggerSize;
	AddChild(new BDragger(frame, this, B_FOLLOW_RIGHT | B_FOLLOW_BOTTOM));
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
ProjectReplicantView::_Reload() {
	if (fStore.LoadAll(fProjects) != B_OK)
		fProjects.clear();
	std::stable_sort(fProjects.begin(), fProjects.end(),
		[](const Project& left, const Project& right) {
			if (left.modified == right.modified)
				return left.name.ICompare(right.name) < 0;
			return left.modified > right.modified;
		});
	Invalidate();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
ProjectReplicantView::_StartWatching() {
	if (fWatching)
		return;
	BEntry directory(fStore.DirectoryPath().Path());
	if (directory.GetNodeRef(&fWatchedDirectory) == B_OK
		&& watch_node(&fWatchedDirectory, B_WATCH_DIRECTORY, BMessenger(this)) == B_OK) {
		fWatching = true;
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
ProjectReplicantView::_StopWatching() {
	if (!fWatching)
		return;
	watch_node(&fWatchedDirectory, B_STOP_WATCHING, BMessenger(this));
	fWatching = false;
}
//---------------------------------------------------------------------------------------------------------------------------------//
