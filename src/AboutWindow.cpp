#include "AboutWindow.h"

#include "Constants.h"

#include <Button.h>
#include <Font.h>
#include <InterfaceDefs.h>
#include <LayoutBuilder.h>
#include <Message.h>
#include <Roster.h>
#include <SeparatorView.h>
#include <Size.h>
#include <String.h>
#include <StringView.h>
#include <TabView.h>
#include <View.h>

namespace {
    class LogoView : public BView {
        public:
            LogoView();
            void Draw(BRect updateRect) override;
    };

    class LinkView : public BStringView {
        public:
            LinkView(const char* name, const char* label, const char* url);
            void AttachedToWindow() override;
            void MouseDown(BPoint point) override;

        private:
            BString fUrl;
    };

LogoView::LogoView() : BView("application icon", B_WILL_DRAW) {
    SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
    SetExplicitMinSize(BSize(64, 64));
    SetExplicitMaxSize(BSize(64, 64));
}
//---------------------------------------------------------------------------------------------------------------------------------//


void LogoView::Draw(BRect updateRect) {
	(void)updateRect;
	BRect bounds(Bounds());
	BRect icon(bounds.InsetByCopy(8, 8));
	SetPenSize(7);
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
	FillEllipse(d, 5, 5);
}
//---------------------------------------------------------------------------------------------------------------------------------//


LinkView::LinkView(const char* name, const char* label, const char* url)
	: BStringView(name, label), fUrl(url) {
	SetToolTip(url);
	BFont font;
	GetFont(&font);
	font.SetFace(font.Face() | B_UNDERSCORE_FACE);
	SetFont(&font);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void LinkView::AttachedToWindow() {
	BStringView::AttachedToWindow();
	SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
	rgb_color background = ui_color(B_PANEL_BACKGROUND_COLOR);
	int32 luminance
		= (299 * background.red + 587 * background.green + 114 * background.blue) / 1000;
	SetHighColor(luminance < 128 ? make_color(105, 190, 255) : make_color(0, 76, 168));
}
//---------------------------------------------------------------------------------------------------------------------------------//


void LinkView::MouseDown(BPoint point) {
	(void)point;
	const char* arguments[] = {fUrl.String()};
	be_roster->Launch("text/html", 1, arguments);
}
//---------------------------------------------------------------------------------------------------------------------------------//


BStringView* Heading(const char* name, const char* text) {
	BStringView* heading = new BStringView(name, text);
	heading->SetFont(be_bold_font);
	return heading;
}
//---------------------------------------------------------------------------------------------------------------------------------//


BView* CreateCreditsView() {
	BView* view = new BView("Credits", B_WILL_DRAW);
	view->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
	BLayoutBuilder::Group<>(view, B_VERTICAL, 5)
		.SetInsets(12)
		.Add(Heading("project heading", "Project"))
		.Add(new BStringView("copyright", "(C)2026 Sikosis"))
		.AddStrut(8)
		.Add(Heading("current heading", "Created:"))
		.Add(new BStringView("current contributor", "Sikosis"))
		.AddStrut(8)
        //.Add(Heading("thanks heading", "Special Thanks"))
		//.Add(new BStringView("haiku thanks", ""))
		.AddGlue();
	return view;
}
//---------------------------------------------------------------------------------------------------------------------------------//


BView* CreateLicencesView() {
	BView* view = new BView("Licences", B_WILL_DRAW);
	view->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
	BLayoutBuilder::Group<>(view, B_VERTICAL, 5)
		.SetInsets(12)
		.Add(Heading("michi licence heading", "michi"))
		.Add(new BStringView("michi licence", "MIT License"))
		.Add(new LinkView("mit link", "MIT License", "https://opensource.org/license/mit"))
		.AddGlue();
	return view;
}
//---------------------------------------------------------------------------------------------------------------------------------//

}

AboutWindow::AboutWindow() : BWindow(BRect(0, 0, 440, 320), kAboutWindowTitle, B_TITLED_WINDOW,	B_NOT_ZOOMABLE | B_NOT_RESIZABLE | B_CLOSE_ON_ESCAPE) {
	BStringView* title = new BStringView("title", "michi");
	BFont titleFont(*be_bold_font);
	titleFont.SetSize(be_plain_font->Size() * 1.8f);
	title->SetFont(&titleFont);

	BStringView* version = new BStringView("version", "Version " MICHI_VERSION);
	BStringView* description
		= new BStringView("description", "A simple Deskbar/Replicant Project Progress Tracker.");

	BTabView* tabs = new BTabView("about tabs", B_WIDTH_FROM_LABEL);
	tabs->AddTab(CreateCreditsView());
	tabs->AddTab(CreateLicencesView());

	BButton* closeButton = new BButton("close", "Close", new BMessage(B_QUIT_REQUESTED));
	closeButton->MakeDefault(true);

	BLayoutBuilder::Group<>(this, B_VERTICAL, 10)
		.SetInsets(16)
		.AddGroup(B_HORIZONTAL, 14)
			.Add(new LogoView())
			.AddGroup(B_VERTICAL, 3)
				.Add(title)
				.Add(version)
				.AddStrut(3)
				.Add(description)
				.Add(new LinkView("source link", "https://github.com/sikosis/michi", "https://github.com/sikosis/michi"))
				.AddGlue()
			.End()
		.End()
		.Add(new BSeparatorView(B_HORIZONTAL))
		.Add(tabs, 1)
		.AddGroup(B_HORIZONTAL)
			.AddGlue()
			.Add(closeButton)
		.End();

	CenterOnScreen();
}
//---------------------------------------------------------------------------------------------------------------------------------//
