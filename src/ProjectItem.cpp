#include "ProjectItem.h"

#include <Font.h>
#include <InterfaceDefs.h>
#include <View.h>

#include <algorithm>

namespace {
rgb_color Unpack(uint32 value)
{
	return {static_cast<uint8>(value >> 24), static_cast<uint8>(value >> 16),
		static_cast<uint8>(value >> 8), static_cast<uint8>(value)};
}
}

ProjectItem::ProjectItem(const Project& project)
	: fProject(project)
{
}

void
ProjectItem::Update(BView* owner, const BFont* font)
{
	BListItem::Update(owner, font);
	SetHeight(std::max(48.0f, Height()));
}

void
ProjectItem::DrawItem(BView* owner, BRect frame, bool)
{
	rgb_color background = ui_color(IsSelected() ? B_LIST_SELECTED_BACKGROUND_COLOR
		: B_LIST_BACKGROUND_COLOR);
	rgb_color text = ui_color(IsSelected() ? B_LIST_SELECTED_ITEM_TEXT_COLOR
		: B_LIST_ITEM_TEXT_COLOR);
	owner->SetHighColor(background);
	owner->FillRect(frame);

	font_height metrics;
	owner->GetFontHeight(&metrics);
	owner->SetHighColor(text);
	owner->DrawString(fProject.name.String(), BPoint(frame.left + 10,
		frame.top + 7 + metrics.ascent));

	BRect bar(frame.left + 10, frame.bottom - 15, frame.right - 52, frame.bottom - 7);
	owner->SetHighColor(tint_color(background, B_DARKEN_1_TINT));
	owner->FillRoundRect(bar, 4, 4);
	BRect filled(bar);
	filled.right = filled.left + bar.Width() * fProject.progress / 100.0f;
	if (filled.right > filled.left) {
		owner->SetHighColor(Unpack(fProject.color));
		owner->FillRoundRect(filled, 4, 4);
	}
	BString percent;
	percent.SetToFormat("%" B_PRId32 "%%", fProject.progress);
	owner->SetHighColor(text);
	owner->DrawString(percent.String(), BPoint(frame.right - 44,
		bar.top + metrics.ascent / 2 + 3));
}
