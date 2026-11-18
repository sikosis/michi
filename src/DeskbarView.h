#ifndef MICHI_DESKBAR_VIEW_H
#define MICHI_DESKBAR_VIEW_H

#include <BeBuild.h>
#include <View.h>

class DeskbarView : public BView {
public:
	explicit DeskbarView(BRect frame);
	explicit DeskbarView(BMessage* archive);

	static BArchivable* Instantiate(BMessage* archive);
	status_t Archive(BMessage* archive, bool deep = true) const override;
	void AttachedToWindow() override;
	void Draw(BRect update) override;
	void MouseDown(BPoint where) override;

private:
	void _Send(uint32 what);
	void _ShowMenu(BPoint where);
};

extern "C" _EXPORT BView* instantiate_deskbar_item(float maxWidth, float maxHeight);

#endif
