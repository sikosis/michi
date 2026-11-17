#include "Project.h"

#include "Constants.h"

#include <Directory.h>
#include <File.h>
#include <FindDirectory.h>
#include <Message.h>
#include <Node.h>
#include <NodeInfo.h>
#include <OS.h>
#include <Volume.h>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <fs_attr.h>
#include <sys/stat.h>
#include <unistd.h>

namespace {
constexpr const char* kNameAttr = "MCHI:name";
constexpr const char* kProgressAttr = "MCHI:progress";
constexpr const char* kColorAttr = "MCHI:color";
constexpr const char* kCreatedAttr = "MCHI:created";
constexpr const char* kModifiedAttr = "MCHI:modified";

int64 Now() {
	return real_time_clock_usecs();
}
//---------------------------------------------------------------------------------------------------------------------------------//


status_t WriteStringAttr(BNode& node, const char* name, const BString& value) {
	ssize_t written = node.WriteAttr(name, B_STRING_TYPE, 0, value.String(),
		value.Length() + 1);
	return written == value.Length() + 1 ? B_OK : (written < 0 ? written : B_ERROR);
}
//---------------------------------------------------------------------------------------------------------------------------------//


status_t ReadStringAttr(BNode& node, const char* name, BString& value) {
	attr_info info;
	status_t status = node.GetAttrInfo(name, &info);
	if (status != B_OK || info.type != B_STRING_TYPE || info.size <= 0 || info.size > 65536)
		return B_BAD_DATA;
	std::vector<char> buffer(static_cast<size_t>(info.size) + 1, 0);
	ssize_t read = node.ReadAttr(name, B_STRING_TYPE, 0, buffer.data(), info.size);
	if (read < 0)
		return read;
	value.SetTo(buffer.data(), static_cast<int32>(strnlen(buffer.data(), read)));
	return B_OK;
}
//---------------------------------------------------------------------------------------------------------------------------------//


template<typename T>
status_t WriteAttr(BNode& node, const char* name, type_code type, const T& value) {
	ssize_t written = node.WriteAttr(name, type, 0, &value, sizeof(T));
	return written == static_cast<ssize_t>(sizeof(T)) ? B_OK
		: (written < 0 ? written : B_ERROR);
}
//---------------------------------------------------------------------------------------------------------------------------------//


template<typename T>
status_t ReadAttr(BNode& node, const char* name, type_code type, T& value) {
	ssize_t read = node.ReadAttr(name, type, 0, &value, sizeof(T));
	return read == static_cast<ssize_t>(sizeof(T)) ? B_OK
		: (read < 0 ? read : B_BAD_DATA);
}
//---------------------------------------------------------------------------------------------------------------------------------//

}

ProjectStore::ProjectStore()
	: fStatus(find_directory(B_USER_DATA_DIRECTORY, &fDirectoryPath, true)) {
	if (fStatus == B_OK)
		fStatus = fDirectoryPath.Append("Michi/Projects");
	if (fStatus == B_OK)
		fStatus = create_directory(fDirectoryPath.Path(), 0755);
}
//---------------------------------------------------------------------------------------------------------------------------------//


status_t
ProjectStore::LoadAll(std::vector<Project>& projects) const {
	projects.clear();
	if (fStatus != B_OK)
		return fStatus;

	BDirectory directory(fDirectoryPath.Path());
	status_t status = directory.InitCheck();
	if (status != B_OK)
		return status;

	entry_ref ref;
	while (directory.GetNextRef(&ref) == B_OK) {
		Project project;
		if (_Read(ref, project) == B_OK)
			projects.push_back(project);
	}
	return B_OK;
}
//---------------------------------------------------------------------------------------------------------------------------------//


status_t
ProjectStore::_Read(const entry_ref& ref, Project& project) const {
	BFile file(&ref, B_READ_ONLY);
	status_t status = file.InitCheck();
	if (status != B_OK)
		return status;

	project.id = ref.name;
	if ((status = ReadStringAttr(file, kNameAttr, project.name)) != B_OK
		|| (status = ReadAttr(file, kProgressAttr, B_INT32_TYPE, project.progress)) != B_OK
		|| (status = ReadAttr(file, kColorAttr, B_UINT32_TYPE, project.color)) != B_OK
		|| (status = ReadAttr(file, kCreatedAttr, B_INT64_TYPE, project.created)) != B_OK
		|| (status = ReadAttr(file, kModifiedAttr, B_INT64_TYPE, project.modified)) != B_OK) {
		return status;
	}
	if (project.name.IsEmpty() || project.progress < 0 || project.progress > 100)
		return B_BAD_DATA;

	off_t size = 0;
	if (file.GetSize(&size) == B_OK && size > 0 && size <= 1024 * 1024) {
		std::vector<char> contents(static_cast<size_t>(size) + 1, 0);
		ssize_t read = file.Read(contents.data(), size);
		if (read > 0)
			project.notes.SetTo(contents.data(), static_cast<int32>(read));
	}
	return B_OK;
}
//---------------------------------------------------------------------------------------------------------------------------------//


status_t
ProjectStore::Save(Project& project) const {
	if (fStatus != B_OK || project.name.IsEmpty() || project.progress < 0
		|| project.progress > 100) {
		return B_BAD_VALUE;
	}

	if (project.id.IsEmpty()) {
		project.id.SetToFormat("project-%" B_PRId64 "-%" B_PRId32,
			Now(), static_cast<int32>(getpid()));
		project.created = Now();
	}
	project.modified = Now();

	BPath path(fDirectoryPath);
	status_t status = path.Append(project.id.String());
	if (status != B_OK)
		return status;
	BFile file(path.Path(), B_READ_WRITE | B_CREATE_FILE | B_ERASE_FILE);
	if ((status = file.InitCheck()) != B_OK)
		return status;
	if (!project.notes.IsEmpty()) {
		ssize_t written = file.Write(project.notes.String(), project.notes.Length());
		if (written != project.notes.Length())
			return written < 0 ? written : B_ERROR;
	}

	if ((status = WriteStringAttr(file, kNameAttr, project.name)) != B_OK
		|| (status = WriteAttr(file, kProgressAttr, B_INT32_TYPE, project.progress)) != B_OK
		|| (status = WriteAttr(file, kColorAttr, B_UINT32_TYPE, project.color)) != B_OK
		|| (status = WriteAttr(file, kCreatedAttr, B_INT64_TYPE, project.created)) != B_OK
		|| (status = WriteAttr(file, kModifiedAttr, B_INT64_TYPE, project.modified)) != B_OK) {
		return status;
	}
	BNodeInfo info(&file);
	return info.SetType(kProjectMimeType);
}
//---------------------------------------------------------------------------------------------------------------------------------//


status_t
ProjectStore::MoveToTrash(const Project& project) const {
	BPath path(fDirectoryPath);
	status_t status = path.Append(project.id.String());
	if (status != B_OK)
		return status;
	BEntry entry(path.Path());
	if ((status = entry.InitCheck()) != B_OK)
		return status;
	entry_ref ref;
	if ((status = entry.GetRef(&ref)) != B_OK)
		return status;
	BVolume volume(ref.device);
	if ((status = volume.InitCheck()) != B_OK)
		return status;
	BPath trashPath;
	if ((status = find_directory(B_TRASH_DIRECTORY, &trashPath, true, &volume)) != B_OK)
		return status;
	BDirectory trash(trashPath.Path());
	return entry.MoveTo(&trash, project.id.String(), false);
}
//---------------------------------------------------------------------------------------------------------------------------------//


SettingsStore::SettingsStore()
	: fStatus(find_directory(B_USER_SETTINGS_DIRECTORY, &fPath, true)) {
	if (fStatus == B_OK) {
		fStatus = fPath.Append("Michi");
		if (fStatus == B_OK)
			fStatus = create_directory(fPath.Path(), 0755);
		if (fStatus == B_OK)
			fStatus = fPath.Append("settings");
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//


status_t
SettingsStore::Load(Settings& settings) const {
	if (fStatus != B_OK)
		return fStatus;
	BFile file(fPath.Path(), B_READ_ONLY);
	if (file.InitCheck() == B_ENTRY_NOT_FOUND)
		return B_OK;
	status_t status = file.InitCheck();
	if (status != B_OK)
		return status;
	BMessage message;
	if ((status = message.Unflatten(&file)) != B_OK)
		return status;
	int32 field;
	if (message.FindInt32("sort_field", &field) == B_OK && field >= 0 && field <= 1)
		settings.sortField = static_cast<SortField>(field);
	message.FindBool("ascending", &settings.ascending);
	message.FindUInt32("default_color", &settings.defaultColor);
	return B_OK;
}
//---------------------------------------------------------------------------------------------------------------------------------//


status_t
SettingsStore::Save(const Settings& settings) const {
	if (fStatus != B_OK)
		return fStatus;
	BFile file(fPath.Path(), B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
	status_t status = file.InitCheck();
	if (status != B_OK)
		return status;
	BMessage message('Mset');
	message.AddInt32("sort_field", static_cast<int32>(settings.sortField));
	message.AddBool("ascending", settings.ascending);
	message.AddUInt32("default_color", settings.defaultColor);
	return message.Flatten(&file);
}
//---------------------------------------------------------------------------------------------------------------------------------//
