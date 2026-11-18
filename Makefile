## Haiku Generic Makefile ##

NAME = michi
TYPE = APP
APP_MIME_SIG = application/x-vnd.michi

SRCS = \
	src/main.cpp \
	src/MichiApp.cpp \
	src/Project.cpp \
	src/ProjectItem.cpp \
	src/ProjectEditor.cpp \
	src/SettingsWindow.cpp \
	src/MainWindow.cpp \
	src/DeskbarView.cpp

RDEFS = resources/Michi.rdef
RSRCS =

LIBS = be $(STDCPPLIBS)
LIBPATHS =
SYSTEM_INCLUDE_PATHS =
LOCAL_INCLUDE_PATHS = src
OPTIMIZE = SOME
LOCALES = en
DEFINES =
WARNINGS = ALL
SYMBOLS =
DEBUGGER =
COMPILER_FLAGS = -std=c++17 -Woverloaded-virtual -Wwrite-strings
LINKER_FLAGS =
APP_VERSION =
DRIVER_PATH =

DEVEL_DIRECTORY := \
	$(shell findpaths -r "makefile_engine" B_FIND_PATH_DEVELOP_DIRECTORY)
include $(DEVEL_DIRECTORY)/etc/makefile-engine
