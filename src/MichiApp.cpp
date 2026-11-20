#include "MichiApp.h"

#include "AboutWindow.h"
#include "Constants.h"
#include "DeskbarView.h"
#include "MainWindow.h"
#include "ProjectEditor.h"
#include "SettingsWindow.h"

#include <Alert.h>
#include <Deskbar.h>
#include <InterfaceDefs.h>
#include <Message.h>
#include <Mime.h>
#include <Roster.h>

#include <cstring>

namespace {
constexpr uint32 kMsgMaybeShowMain = 'dflt';

void AddAttribute(BMessage& info, const char* name, const char* publicName,
	type_code type, int32 width, bool editable) {
	info.AddString("attr:name", name);
	info.AddString("attr:public_name", publicName);
	info.AddInt32("attr:type", type);
	info.AddInt32("attr:width", width);
	info.AddInt32("attr:alignment", B_ALIGN_LEFT);
	info.AddBool("attr:extra", false);
	info.AddBool("attr:viewable", true);
	info.AddBool("attr:editable", editable);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void RegisterProjectAttributes(BMimeType& type) {
	BMessage info;
	AddAttribute(info, "MCHI:name", "Project", B_STRING_TYPE, 180, true);
	AddAttribute(info, "MCHI:progress", "Progress", B_INT32_TYPE, 70, true);
	AddAttribute(info, "MCHI:created", "Created", B_INT64_TYPE, 120, false);
	AddAttribute(info, "MCHI:modified", "Modified", B_INT64_TYPE, 120, false);
	type.SetAttrInfo(&info);
}
//---------------------------------------------------------------------------------------------------------------------------------//

}

MichiApp::MichiApp() : BApplication(kAppSignature), fMainWindow(nullptr), fInitialMessageHandled(false) {
	fSettingsStore.Load(fSettings);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void MichiApp::ReadyToRun() {
	BMimeType app(kAppSignature);
	if (!app.IsInstalled())
		app.Install();
	app_info info;
	if (GetAppInfo(&info) == B_OK)
		app.SetAppHint(&info.ref);
	BMimeType project(kProjectMimeType);
	if (!project.IsInstalled()) {
		project.Install();
		project.SetShortDescription("Michi - walk the path");
		project.SetLongDescription("michi project progress record");
		project.SetPreferredApp(kAppSignature);
	}
	RegisterProjectAttributes(project);
	_InstallDeskbarItem();
	PostMessage(kMsgMaybeShowMain);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void MichiApp::ArgvReceived(int32, char**) {
}
//---------------------------------------------------------------------------------------------------------------------------------//


void MichiApp::_InstallDeskbarItem() {
	BDeskbar deskbar;
	if (!deskbar.IsRunning())
		return;
	status_t status = B_OK;
	if (deskbar.HasItem(kDeskbarItemName))
		status = deskbar.RemoveItem(kDeskbarItemName);
	app_info info;
	if (status == B_OK)
		status = GetAppInfo(&info);
	if (status == B_OK)
		status = deskbar.AddItem(&info.ref);
	if (status != B_OK) {
		BString error("Could not install the Deskbar item: ");
		error << strerror(status);
		(new BAlert("Michi", error.String(), "OK"))->Go();
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//


void MichiApp::_ShowMain() {
	if (fMainWindow != nullptr) {
		if (fMainWindow->Lock()) {
			fMainWindow->Reload();
			fMainWindow->Show();
			fMainWindow->Activate(true);
			fMainWindow->Unlock();
		}
		return;
	}
	fMainWindow = new MainWindow(fStore, fSettings);
	fMainWindow->Show();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void MichiApp::_ShowSettings() {
	for (int32 index = 0; index < CountWindows(); ++index) {
		SettingsWindow* window = dynamic_cast<SettingsWindow*>(WindowAt(index));
		if (window != nullptr && window->Lock()) {
			window->SetSettings(fSettings);
			if (window->IsHidden())
				window->Show();
			window->Activate(true);
			window->Unlock();
			return;
		}
	}
	(new SettingsWindow(fSettings, BMessenger(this)))->Show();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void MichiApp::AboutRequested() {
	(new AboutWindow())->Show();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void MichiApp::MessageReceived(BMessage* message) {
	switch (message->what) {
		case kMsgMaybeShowMain:
			if (!fInitialMessageHandled)
				_ShowMain();
			break;
		case kMsgShowMain:
			fInitialMessageHandled = true;
			_ShowMain();
			break;
		case kMsgShowSettings:
			fInitialMessageHandled = true;
			_ShowSettings();
			break;
		case kMsgShowAbout:
		case B_ABOUT_REQUESTED:
			fInitialMessageHandled = true;
			AboutRequested();
			break;
		case kMsgSaveProject: _SaveProject(message); break;
		case kMsgSettingsChanged: _ApplySettings(message); break;
		default: BApplication::MessageReceived(message); break;
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//


void MichiApp::_SaveProject(BMessage* message) {
	Project project;
	const char* value;
	if (message->FindString("id", &value) == B_OK) project.id = value;
	if (message->FindString("name", &value) == B_OK) project.name = value;
	if (message->FindString("notes", &value) == B_OK) project.notes = value;
	message->FindInt32("progress", &project.progress);
	message->FindUInt32("color", &project.color);
	message->FindInt64("created", &project.created);
	Project existing;
	status_t status = fStore.FindByName(project.name, existing);
	if (status == B_OK && existing.id != project.id) {
		BString prompt;
		prompt.SetToFormat("A project named ‘%s’ already exists. Edit the existing project instead?",
			project.name.String());
		BAlert* alert = new BAlert("Duplicate Project", prompt.String(), "Cancel",
			"Edit Existing", nullptr, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
		if (alert->Go() == 1)
			(new ProjectEditor(&existing, fSettings.defaultColor, BMessenger(this)))->Show();
		return;
	}
	status = fStore.Save(project);
	if (status != B_OK) {
		BString error("Could not save the project: ");
		error << strerror(status);
		(new BAlert("Michi", error.String(), "OK"))->Go();
		return;
	}
	if (fMainWindow != nullptr)
		BMessenger(fMainWindow).SendMessage(kMsgProjectsChanged);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void MichiApp::_ApplySettings(BMessage* message) {
	int32 field;
	if (message->FindInt32("sort_field", &field) == B_OK)
		fSettings.sortField = static_cast<SortField>(field);
	message->FindBool("ascending", &fSettings.ascending);
	message->FindUInt32("default_color", &fSettings.defaultColor);
	status_t status = fSettingsStore.Save(fSettings);
	if (status != B_OK) {
		BString error("Could not save settings: ");
		error << strerror(status);
		(new BAlert("Michi", error.String(), "OK"))->Go();
	}
	if (fMainWindow != nullptr)
		BMessenger(fMainWindow).SendMessage(kMsgSettingsChanged);
}
//---------------------------------------------------------------------------------------------------------------------------------//
