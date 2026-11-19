#include "MainWindow.h"

#include "Constants.h"
#include "ProjectEditor.h"
#include "ProjectItem.h"
#include "ProjectReplicantView.h"

#include <Alert.h>
#include <Application.h>
#include <Button.h>
#include <LayoutBuilder.h>
#include <ListView.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <NodeMonitor.h>
#include <OS.h>
#include <PopUpMenu.h>
#include <ScrollView.h>
#include <StringView.h>

#include <algorithm>
#include <cstring>

namespace {
enum : int32 {
	kFilterAll = 0,
	kFilterUnfinished,
	kFilterFinished,
	kFilterNewest,
	kFilterOldest
};

constexpr int64 kMonthUsecs = 30LL * 24 * 60 * 60 * 1000000;

BMenuItem*
FilterItem(const char* label, int32 filter) {
	BMessage* message = new BMessage(kMsgFilterProjects);
	message->AddInt32("filter", filter);
	return new BMenuItem(label, message);
}
//---------------------------------------------------------------------------------------------------------------------------------//
}

MainWindow::MainWindow(ProjectStore& store, Settings& settings)
	: BWindow(BRect(0, 0, 620, 460), kAppTitle, B_TITLED_WINDOW,
		B_AUTO_UPDATE_SIZE_LIMITS), fStore(store), fSettings(settings), fWatching(false),
		fEmptyStateInitialised(false), fShowingEmpty(false), fFilter(kFilterAll) {
	fList = new BListView("projects", B_SINGLE_SELECTION_LIST);
	fList->SetInvocationMessage(new BMessage(kMsgEditProject));
	fEmpty = new BStringView("empty", "No projects yet. Choose New to begin.");
	fEmpty->SetAlignment(B_ALIGN_CENTER);
	BButton* add = new BButton("New", new BMessage(kMsgNewProject));
	BButton* replicant = new BButton("Replicant", new BMessage(kMsgShowReplicant));
	fEdit = new BButton("Edit", new BMessage(kMsgEditProject));
	fDelete = new BButton("Delete", new BMessage(kMsgDeleteProject));
	BPopUpMenu* filterMenu = new BPopUpMenu("project filter");
	filterMenu->SetRadioMode(true);
	filterMenu->SetLabelFromMarked(true);
	filterMenu->AddItem(FilterItem("All", kFilterAll));
	filterMenu->AddItem(FilterItem("Unfinished", kFilterUnfinished));
	filterMenu->AddItem(FilterItem("Finished", kFilterFinished));
	filterMenu->AddSeparatorItem();
	filterMenu->AddItem(FilterItem("Newest", kFilterNewest));
	filterMenu->AddItem(FilterItem("Oldest", kFilterOldest));
	filterMenu->ItemAt(0)->SetMarked(true);
	filterMenu->SetTargetForItems(this);
	BMenuField* filterField = new BMenuField("Filter:", filterMenu);

	BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_DEFAULT_SPACING)
		.SetInsets(B_USE_WINDOW_INSETS)
		.AddGroup(B_HORIZONTAL)
			.Add(new BStringView("heading", "Projects"))
			.AddGlue()
			.Add(replicant)
			.Add(filterField)
		.End()
		.Add(fEmpty)
		.Add(new BScrollView("project_scroll", fList, 0, false, true), 1)
		.AddGroup(B_HORIZONTAL)
			.Add(add)
			.AddGlue()
			.Add(fEdit)
			.Add(fDelete)
		.End();
	SetSizeLimits(430, B_SIZE_UNLIMITED, 280, B_SIZE_UNLIMITED);
	CenterOnScreen();
	BEntry directoryEntry(fStore.DirectoryPath().Path());
	if (directoryEntry.GetNodeRef(&fWatchedDirectory) == B_OK
		&& watch_node(&fWatchedDirectory, B_WATCH_DIRECTORY, BMessenger(this)) == B_OK) {
		fWatching = true;
	}
	Reload();
}
//---------------------------------------------------------------------------------------------------------------------------------//


MainWindow::~MainWindow() {
	if (fWatching)
		watch_node(&fWatchedDirectory, B_STOP_WATCHING, BMessenger(this));
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
MainWindow::Reload() {
	fList->MakeEmpty();
	status_t status = fStore.LoadAll(fProjects);
	if (status != B_OK) {
		BString message("Michi could not read its Project Folder: ");
		message << strerror(status);
		(new BAlert("Michi", message.String(), "OK"))->Go();
	}
	bool hasProjects = !fProjects.empty();
	fProjects.erase(std::remove_if(fProjects.begin(), fProjects.end(),
		[this](const Project& project) { return !_MatchesFilter(project); }), fProjects.end());
	fEmpty->SetText(hasProjects
		? "No projects match this filter."
		: "No projects yet. Choose New to begin.");
	auto less = [this](const Project& a, const Project& b) {
		int64 left = fSettings.sortField == SortField::Created ? a.created : a.modified;
		int64 right = fSettings.sortField == SortField::Created ? b.created : b.modified;
		if (left == right)
			return a.name.ICompare(b.name) < 0;
		return fSettings.ascending ? left < right : left > right;
	};
	std::stable_sort(fProjects.begin(), fProjects.end(), less);
	for (const Project& project : fProjects)
		fList->AddItem(new ProjectItem(project));
	_UpdateEmptyState();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
MainWindow::MessageReceived(BMessage* message) {
	switch (message->what) {
		case B_NODE_MONITOR: Reload(); break;
		case kMsgNewProject: _OpenEditor(nullptr); break;
		case kMsgEditProject: {
			Project* project = _SelectedProject();
			if (project != nullptr)
				_OpenEditor(project);
			break;
		}
		case kMsgDeleteProject: _DeleteSelected(); break;
		case kMsgShowReplicant: {
			BWindow* window = new BWindow(BRect(0, 0, 340, 420), kReplicantWindowTitle,
				B_TITLED_WINDOW, B_NOT_ZOOMABLE | B_NOT_RESIZABLE | B_CLOSE_ON_ESCAPE);
			window->AddChild(new ProjectReplicantView(window->Bounds()));
			window->CenterOnScreen();
			window->Show();
			break;
		}
		case kMsgFilterProjects:
			if (message->FindInt32("filter", &fFilter) == B_OK)
				Reload();
			break;
		case kMsgProjectsChanged:
		case kMsgSettingsChanged: Reload(); break;
		default: BWindow::MessageReceived(message); break;
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool
MainWindow::_MatchesFilter(const Project& project) const {
	switch (fFilter) {
		case kFilterUnfinished: return project.progress < 100;
		case kFilterFinished: return project.progress == 100;
		case kFilterNewest: return project.modified >= real_time_clock_usecs() - kMonthUsecs;
		case kFilterOldest: return project.modified < real_time_clock_usecs() - kMonthUsecs;
		default: return true;
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//


Project*
MainWindow::_SelectedProject() {
	int32 index = fList->CurrentSelection();
	return index >= 0 && index < static_cast<int32>(fProjects.size())
		? &fProjects[static_cast<size_t>(index)] : nullptr;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
MainWindow::_OpenEditor(const Project* project) {
	(new ProjectEditor(project, fSettings.defaultColor, BMessenger(be_app)))->Show();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
MainWindow::_DeleteSelected() {
	Project* project = _SelectedProject();
	if (project == nullptr)
		return;
	BString prompt;
	prompt.SetToFormat("Move ‘%s’ to Trash?", project->name.String());
	BAlert* alert = new BAlert("Delete Project", prompt.String(), "Cancel", "Move to Trash",
		nullptr, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
	if (alert->Go() != 1)
		return;
	status_t status = fStore.MoveToTrash(*project);
	if (status != B_OK) {
		BString error("Could not move the project to Trash: ");
		error << strerror(status);
		(new BAlert("Michi", error.String(), "OK"))->Go();
	}
	Reload();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
MainWindow::_UpdateEmptyState() {
	bool empty = fProjects.empty();
	if (fEmptyStateInitialised && empty == fShowingEmpty)
		return;

	if (empty) {
		if (fEmptyStateInitialised)
			fEmpty->Show();
		fList->Hide();
	} else {
		fEmpty->Hide();
		if (fEmptyStateInitialised)
			fList->Show();
	}
	fShowingEmpty = empty;
	fEmptyStateInitialised = true;
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool
MainWindow::QuitRequested() {
	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
}
//---------------------------------------------------------------------------------------------------------------------------------//
