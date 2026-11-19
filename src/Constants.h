#ifndef MICHI_CONSTANTS_H
#define MICHI_CONSTANTS_H

#include <SupportDefs.h>

#define MICHI_VERSION "0.3"

constexpr const char* kAppSignature = "application/x-vnd.michi";
constexpr const char* kProjectMimeType = "application/x-vnd.michi-project";
constexpr const char* kDeskbarItemName = "Michi";
constexpr const char* kAppTitle = "michi v" MICHI_VERSION;
constexpr const char* kSettingsWindowTitle = "michi v" MICHI_VERSION " — Settings";
constexpr const char* kNewProjectWindowTitle = "michi v" MICHI_VERSION " — New Project";
constexpr const char* kEditProjectWindowTitle = "michi v" MICHI_VERSION " — Edit Project";
constexpr const char* kAboutWindowTitle = "michi v" MICHI_VERSION " — About";
constexpr const char* kReplicantWindowTitle = "michi v" MICHI_VERSION " — Replicant";

enum : uint32 {
	kMsgShowMain = 'shmn',
	kMsgShowSettings = 'shst',
	kMsgShowAbout = 'shab',
	kMsgDeskbarQuit = 'dbqt',
	kMsgNewProject = 'newp',
	kMsgEditProject = 'edtp',
	kMsgDeleteProject = 'delp',
	kMsgFilterProjects = 'fltr',
	kMsgShowReplicant = 'repl',
	kMsgSaveProject = 'savp',
	kMsgProjectsChanged = 'prch',
	kMsgSaveSettings = 'svst',
	kMsgSettingsChanged = 'stch'
};

#endif
