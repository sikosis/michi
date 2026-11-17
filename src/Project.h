#ifndef MICHI_PROJECT_H
#define MICHI_PROJECT_H

#include <Entry.h>
#include <Path.h>
#include <String.h>
#include <SupportDefs.h>

#include <vector>

struct Project {
	BString id;
	BString name;
	BString notes;
	int32 progress = 0;
	uint32 color = 0x3b82f6ff;
	int64 created = 0;
	int64 modified = 0;
};

enum class SortField : int32 { Created = 0, Modified = 1 };

struct Settings {
	SortField sortField = SortField::Modified;
	bool ascending = true;
	uint32 defaultColor = 0x3b82f6ff;
};

class ProjectStore {
public:
	ProjectStore();

	status_t InitCheck() const {
		return fStatus;
	}
	//---------------------------------------------------------------------------------------------------------------------------------//

	const BPath& DirectoryPath() const {
		return fDirectoryPath;
	}
	//---------------------------------------------------------------------------------------------------------------------------------//

	status_t LoadAll(std::vector<Project>& projects) const;
	status_t Save(Project& project) const;
	status_t MoveToTrash(const Project& project) const;

private:
	status_t _Read(const entry_ref& ref, Project& project) const;
	BPath fDirectoryPath;
	status_t fStatus;
};

class SettingsStore {
public:
	SettingsStore();
	status_t Load(Settings& settings) const;
	status_t Save(const Settings& settings) const;

private:
	BPath fPath;
	status_t fStatus;
};

#endif
