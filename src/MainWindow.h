#ifndef MICHI_MAIN_WINDOW_H
#define MICHI_MAIN_WINDOW_H

#include "Project.h"

#include <Node.h>
#include <Window.h>

#include <vector>

class BButton;
class BListView;
class BStringView;

class MainWindow : public BWindow {
public:
	MainWindow(ProjectStore& store, Settings& settings);
	~MainWindow() override;
	void MessageReceived(BMessage* message) override;
	bool QuitRequested() override;
	void Reload();

private:
	Project* _SelectedProject();
	void _OpenEditor(const Project* project);
	void _DeleteSelected();
	void _UpdateEmptyState();

	ProjectStore& fStore;
	Settings& fSettings;
	std::vector<Project> fProjects;
	BListView* fList;
	BStringView* fEmpty;
	BButton* fEdit;
	BButton* fDelete;
	node_ref fWatchedDirectory;
	bool fWatching;
};

#endif
