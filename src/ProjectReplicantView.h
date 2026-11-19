#ifndef MICHI_PROJECT_REPLICANT_VIEW_H
#define MICHI_PROJECT_REPLICANT_VIEW_H

#include "Project.h"

#include <BeBuild.h>
#include <Node.h>
#include <View.h>

#include <vector>

class _EXPORT ProjectReplicantView : public BView {
public:
	explicit ProjectReplicantView(BRect frame);
	explicit ProjectReplicantView(BMessage* archive);
	~ProjectReplicantView() override;

	static BArchivable* Instantiate(BMessage* archive);
	status_t Archive(BMessage* archive, bool deep = true) const override;
	void AttachedToWindow() override;
	void Draw(BRect updateRect) override;
	void MessageReceived(BMessage* message) override;
	void MouseDown(BPoint where) override;

private:
	void _AddDragger();
	void _Reload();
	void _StartWatching();
	void _StopWatching();

	ProjectStore fStore;
	std::vector<Project> fProjects;
	node_ref fWatchedDirectory;
	bool fWatching;
};

#endif
