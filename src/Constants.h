#ifndef MICHI_CONSTANTS_H
#define MICHI_CONSTANTS_H

#include <SupportDefs.h>

constexpr const char* kAppSignature = "application/x-vnd.michi";
constexpr const char* kProjectMimeType = "application/x-vnd.michi-project";
constexpr const char* kDeskbarItemName = "Michi";

enum : uint32 {
	kMsgShowMain = 'shmn',
	kMsgShowSettings = 'shst',
	kMsgShowAbout = 'shab',
	kMsgDeskbarQuit = 'dbqt',
	kMsgNewProject = 'newp',
	kMsgEditProject = 'edtp',
	kMsgDeleteProject = 'delp',
	kMsgSaveProject = 'savp',
	kMsgProjectsChanged = 'prch',
	kMsgSaveSettings = 'svst',
	kMsgSettingsChanged = 'stch'
};

#endif
